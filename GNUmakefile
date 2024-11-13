CC      := gcc
MACHINE := $(shell uname -m)
ARCH    := $(shell $(CC) -dumpmachine | cut -f1 -d-)
# to detect mingw
TARGET  := $(shell $(CC) -dumpmachine | cut -f3 -d-)
TIMEOUT_1m := timeout --preserve-status 1m
TIMEOUT_4m := timeout 4m
ifeq (, $(shell command -v timeout))
  TIMEOUT_1m =
  TIMEOUT_4m =
endif
BINDIR   := bin
ALGOSINC := $(wildcard source/algos/include/*.h)
SRCINC   := $(wildcard source/*.h)
ifneq ($(ARCH),x86_64)
  CFLAGS  := -O3 -Wall
  NON_SSE = source/algos/epsm.c source/algos/ssecp.c source/algos/ssef.c
  ALGOSRC := $(filter-out $(NON_SSE),$(wildcard source/algos/*.c))
else
  CFLAGS  := -O3 -march=native -mtune=native -Wall -Wfatal-errors
  ifeq ($(SANITIZE),1)
    BINDIR = bin/asan
    CFLAGS += -g -Wextra -fsanitize=address,undefined -DBINDIR=\"$(BINDIR)\"
  endif
  ifeq ($(FUZZ),1)
    CC = afl-clang-lto
    BINDIR = bin/fuzz
    CFLAGS = -Og -g -Isource/algos -fsanitize=address,undefined -march=native -mtune=native -DFUZZ -DBINDIR=\"$(BINDIR)\"
  endif
  ALGOSRC := $(wildcard source/algos/*.c)
endif
ifneq ($(ASSERT),1)
  ifneq ($(SANITIZE),1)
    ifneq ($(FUZZ),1)
      CFLAGS += -DNDEBUG
    endif
  endif
endif
ifneq ($(ARCH),$(MACHINE))
  ifeq ($(BINDIR),bin)
    BINDIR = bin/$(ARCH)
    CFLAGS += -DBINDIR=\"$(BINDIR)\"
  endif
  DRV = qemu-$(ARCH)
else
  # debian nonsense calling linux gnu
  ifneq ($(TARGET),gnu)
    OS := $(shell uname -s | tr A-Z a-z)
    ifeq ($(OS),darwin)
      ifeq ($(shell echo $(TARGET)|cut -c1-6),darwin)
	TARGET=darwin
      endif
    endif
    ifneq ($(TARGET),$(OS))
      ifeq ($(BINDIR),bin)
        BINDIR = bin/$(TARGET)
        CFLAGS += -DBINDIR=\"$(BINDIR)\"
      endif
      ifeq ($(TARGET),mingw32)
        ALGOSRC := $(filter-out source/algos/libc1.c,$(wildcard source/algos/*.c))
        DRV = wine
      else
	DRV = qemu-$(ARCH)
      endif
    endif
  endif
endif
ALLSRC  = $(ALGOSRC) $(wildcard source/*.c) $(SRCINC) $(ALGOSINC)
BINS    = $(patsubst source/algos/%,$(BINDIR)/%,$(patsubst %.c,%,$(ALGOSRC)))
ifeq ($(SANITIZE),1)
  TESTBIN = test-asan
  SMARTBIN = smart-asan
  SELECTBIN = select-asan
  HELPERS = $(SMARTBIN) $(TESTBIN) $(SELECTBIN) compilesm-asan show textgen algocfg
else
ifeq ($(FUZZ),1)
  TESTBIN = test-fuzz
  SMARTBIN = smart-fuzz
  SELECTBIN = select
  HELPERS = $(SMARTBIN) $(TESTBIN) $(SELECTBIN) compilesm-fuzz show textgen algocfg
else
  TESTBIN = test
  SMARTBIN = smart
  SELECTBIN = select
  HELPERS = $(SMARTBIN) $(TESTBIN) $(SELECTBIN) compilesm show textgen algocfg
endif
endif
TESTS := $(shell shuf -n 10 good.lst)
ifeq ($(TESTS),)
  TESTS = hor mp kmp musl1 tbm so ssm qf33 twfr3 fndm
endif
COMPILE = $(CC) $(CFLAGS)

all: $(BINS) $(HELPERS) good.lst asan.lst

$(BINDIR)/%: source/algos/%.c $(ALGOSINC) GNUmakefile
	@test -d $(BINDIR) || mkdir $(BINDIR)
	$(CC) $(CFLAGS) $< -o $@
./%-fuzz: source/%.c $(SRCINC) source/algos/include/shmids.h GNUmakefile
	$(CC) $(CFLAGS) -DFUZZ $< -std=gnu99 -o $@ -lm
./%-asan: source/%.c $(SRCINC) source/algos/include/shmids.h GNUmakefile
	$(CC) $(CFLAGS) $< -std=gnu99 -o $@ -lm
./%: source/%.c $(SRCINC) source/algos/include/shmids.h GNUmakefile
	$(CC) $(CFLAGS) $< -std=gnu99 -o $@ -lm
$(SELECTBIN): source/selectAlgo.c $(SRCINC) GNUmakefile
	$(CC) $(CFLAGS) $< -o $@
verify/%.vfy: source/algos/%.c $(ALGOSINC) algocfg GNUmakefile
	b=`basename $@ .vfy`; echo -n "cmd="; ./algocfg $$b cbmc; \
	  cmd=`./algocfg $$b cbmc`; \
	  echo $$cmd $(CBMC_ARGS) $< > $@; \
	  $$cmd $(CBMC_ARGS) $< 2>&1 | tee -a $@
	if grep UNSATISFIABLE $@ >/dev/null; then \
	  echo | tee -a $@; b=`basename $@ .vfy`; \
	  echo goto-analyzer -Isource/algos -DCBMC --verify --recursive-interprocedural source/algos/$$b.c | tee -a $@; \
	  goto-analyzer -Isource/algos -DCBMC --verify --recursive-interprocedural source/algos/$$b.c | tee -a $@; \
          $(MAKE) verify/$$b.vfy-trace; \
	fi
CMBC_TRACE_ARGS = --trace --reachability-slice-fb
verify/%.vfy-trace: source/algos/%.c $(ALGOSINC) algocfg GNUmakefile
	b=`basename $@ .vfy-trace`; echo -n "cmd="; ./algocfg $$b cbmc; \
	  cmd=`./algocfg $$b cbmc`; \
	  echo $$cmd $(CMBC_TRACE_ARGS) $(CBMC_ARGS) $< > $@; \
	  $$cmd $(CMBC_TRACE_ARGS) $(CBMC_ARGS) $< 2>&1 | tee -a $@
	if grep UNSATISFIABLE $@ >/dev/null; then \
	  echo | tee -a $@; b=`basename $@ .vfy`; \
	  echo goto-analyzer -Isource/algos -DCBMC --verify --recursive-interprocedural source/algos/$$b.c | tee -a $@; \
	  goto-analyzer -Isource/algos -DCBMC --verify --recursive-interprocedural source/algos/$$b.c | tee -a $@; \
	fi

.PHONY: check clean all lint verify check-verify verify-trace fmt cppcheck clang-tidy fuzz
check: all
	-cp source/algorithms.lst source/algorithms.lst.bak
	$(DRV) ./$(SELECTBIN) -all
	$(DRV) ./$(SELECTBIN) -which | grep br
	-cp $(BINDIR)/br $(BINDIR)/br1
	$(DRV) ./$(SELECTBIN) -add br1
	-rm $(BINDIR)/br1
	$(DRV) ./$(SELECTBIN) -none $(TESTS)
	$(DRV) ./$(SMARTBIN) -text rand4:rand32 -plen 2 4
	$(DRV) ./$(SMARTBIN) -simple abab chbjhxsscsjndwkjnjdnwelabakdlkewdkklewlkdewlkdnewknabdewab
	for t in $(TESTS); do echo $$t -nv; $(DRV) ./$(TESTBIN) $$t -nv; done
	for t in $(TESTS); do echo $$t -nv rand2 2; $(DRV) ./$(TESTBIN) $$t -nv rand2 2; done
	$(DRV) ./$(SELECTBIN) -all block bmh2 bmh4 dfdm sbdm faoso2 blim ssecp
	-mv algorithms.lst.bak algorithms.lst
lint: algocfg cppcheck clang-tidy sanitizer.log
# for newer cppcheck
#CPPCHECK_ARGS="-j4 --enable=warning,portability --inline-suppr --check-level=exhaustive"
CPPCHECK_ARGS = -j4 --enable=warning,portability --inline-suppr
cppcheck:
	cppcheck $(CPPCHECK_ARGS) source/*.c source/algos/*.c
compile_commands.json: GNUmakefile
	+$(MAKE) clean
	bear -- $(MAKE)
clang-tidy.log: compile_commands.json $(ALLSRC)
	clang-tidy source/*.c source/algos/*.c | sed -e"s,$$PWD/,," | tee clang-tidy.log
clang-tidy: clang-tidy.log
sanitizer.log: $(ALLSRC)
	-rm -f sanitizer.log 2>/dev/null
	-./sanitizer.sh 2>sanitizer.log
tests.lst: $(ALLSRC)
	for t in `cat algos.lst`; do ./test "$$t"; done | tee $@
good.lst: algocfg
	./algocfg good | tr ' ' '\n' >$@
asan.lst: algocfg
	./algocfg good 0 | perl -nle'%bad=map{$$_=>1}split/ /;for(split/ /,qx"./algocfg asan"){print $$_ if $$_ ne "\n" and !$$bad{$$_}}' >$@

ifeq (esbmc, $(CBMC))
CBMC_ARGS = -Isource/algos -DCBMC -DESBMC
else
CBMC_ARGS = -Isource/algos -DCBMC --bounds-check --pointer-check --memory-leak-check \
  --div-by-zero-check --signed-overflow-check --unsigned-overflow-check   \
  --pointer-overflow-check --conversion-check --undefined-shift-check     \
  --float-overflow-check --nan-check --enum-range-check
endif
#CBMC_VERSION := $(shell cbmc --version | cut -c1-3)
#CBMC_ARGS += $(shell if test "`echo "$$CBMC_VERSION >= 5.13" | bc`" = 1; then \
#	echo "--pointer-primitive-check"; fi)
# cbmc 5.12.1: --pointer-primitive-check
# UNSATISFIABLE: passes, but needs more depth or builtins (nested loops => memset)
FAIL_VERIFY := $(shell ./algocfg VFY_FAIL)
TIMEOUT_VERIFY := $(shell ./algocfg VFY_TIMEOUT)
NON_CBMC_SRC   = $(addsuffix .c, $(addprefix source/algos/,$(TIMEOUT_VERIFY)))
verify: verify/verify.log
verify/verify.log: $(ALGOSRC) $(ALGOSINC) algocfg GNUmakefile
	for c in $(ALGOSRC); do \
	  echo $$c; b=`basename $$c .c`; ./algocfg $$b cbmc; cmd=`./algocfg $$b cbmc`; \
	  echo $$cmd $(CBMC_ARGS) $$c; \
	  $$cmd $(CBMC_ARGS) $$c || \
            (echo $$cmd $(CBMC_ARGS) "$$c FAILED"; \
	    test $(( `./algocfg $$b VFY_FAIL` + `./algocfg $$b VFY_TIMEOUT` )) -gt 0 || exit 1); \
	done | tee verify/verify.log
	for b in `./algocfg UNSATISFIABLE` `./algocfg VFY_TIMEOUT`; do \
	  echo goto-analyzer -DCBMC --verify --recursive-interprocedural source/algos/$$b.c; \
	  goto-analyzer -DCBMC --verify --recursive-interprocedural source/algos/$$b.c; \
	done | tee -a verify/verify.log;
check-verify: algocfg GNUmakefile
	for c in $(addsuffix .c, $(addprefix source/algos/,$(filter-out $(TIMEOUT_VERIFY),$(TESTS)))); \
	do \
	  echo $$c; b=`basename $$c .c`; ./algocfg $$b cbmc; \
	  cmd=`./algocfg $$b cbmc`; \
	  echo $$cmd $(CBMC_ARGS) $$c; \
	  $$cmd $(CBMC_ARGS) $$c || \
            (echo $$cmd $(CBMC_ARGS) "$$c FAILED"; \
	     test $(( `./algocfg $$b VFY_FAIL` + `./algocfg $$b VFY_TIMEOUT` )) -gt 0 || exit 1); \
	done
# prints the violations
verify-trace: verify/trace.log
verify/trace.log: algocfg $(addsuffix .c, $(addprefix source/algos/,$(FAIL_VERIFY))) $(ALGOSINC) GNUmakefile
	for c in $(addsuffix .c, $(addprefix source/algos/,$(FAIL_VERIFY))); do \
	  echo $$c; b=`basename $$c .c`; ./algocfg $$b cbmc; \
	  cmd=`./algocfg $$b cbmc`; \
	  echo $$cmd --trace $(CBMC_ARGS) $$c; \
	  $$cmd --trace $(CBMC_ARGS) $$c || \
            (echo $$cmd --trace $(CBMC_ARGS) " $$c FAILED"; \
	     test $(( `./algocfg $$b VFY_FAIL` + `./algocfg $$b VFY_TIMEOUT` )) -gt 0 || exit 1); \
	done | tee verify/trace.log
fuzz: test-fuzz algocfg GNUmakefile
	echo '!#/bin/sh' >fuzz.sh
	-for c in $(ALGOSRC); do \
	  b="`basename $$c .c`"; echo $(MAKE) $$b.fuzz >>fuzz.sh; \
	done; chmod +x fuzz.sh; sh ./fuzz.sh
%.fuzz: algocfg source/algos/%.c $(ALGOSINC) GNUmakefile
	b=`basename $@ .fuzz`; \
	  $(MAKE) FUZZ=1 bin/fuzz/$$b; \
	  timeout 30s afl-fuzz -i data/midimusic -o fuzz/$$b -- bin/fuzz/$$b; \
	  for c in fuzz/$$b/default/crashes/id\:*; do \
	    if [ -n "$c" ]; then xxd -i $c fuzz/$$b/id$(basename "$c" | cut -c4-9).h; fi; \
	  done; \
	  ps xw|grep 'bin/[f]uzz' |cut -c1-8|xargs kill -9
.PHONY: fuzz-repro
fuzz-repro:
	$(MAKE) SANITIZE=1
	for i in fuzz/*/default/crashes/id*; do \
	  ./fuzz-repro.sh "$$i"; done

# emacs flymake-mode
check-syntax:
	test -n "$(CHK_SOURCES)" && \
	  $(COMPILE) -c -o /dev/null -S $(CHK_SOURCES)
.PHONY: check-syntax
fmt:
	clang-format -i `find source -name \*.c -o -name \*.h`
clean:
	-rm -f $(BINS) $(HELPERS) 2>/dev/null
TAGS: $(ALLSRC)
	-rm -f TAGS 2>/dev/null
	find . \( -name \*.c -o -name \*.h \) -exec etags -a --language=c \{\} \;
