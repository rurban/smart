CC      := gcc
MACHINE := $(shell uname -m)
ARCH    := $(shell $(CC) -dumpmachine | cut -f1 -d-)
# to detect mingw
TARGET  := $(shell $(CC) -dumpmachine | cut -f3 -d-)
TIMEOUT_1m := timeout 1m
TIMEOUT_4m := timeout 4m
ifeq (, $(shell which timeout))
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
    CC = /usr/bin/afl-clang-lto
    BINDIR = bin/fuzz
    CFLAGS = -O1 -march=native -mtune=native -g -DFUZZ -DBINDIR=\"$(BINDIR)\"
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
  BINDIR = bin/$(ARCH)
  CFLAGS += -DBINDIR=\"$(BINDIR)\"
  DRV = qemu-$(ARCH)
else
  # debian nonsense calling linux gnu
  ifneq ($(TARGET),gnu)
    ifneq ($(TARGET),$(shell uname -s | tr A-Z a-z))
      BINDIR = bin/$(TARGET)
      CFLAGS += -DBINDIR=\"$(BINDIR)\"
      ifeq ($(TARGET),mingw32)
        ALGOSRC := $(filter-out source/algos/libc1.c,$(wildcard source/algos/*.c))
        DRV = wine
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

all: $(BINS) $(HELPERS) good.lst asan.lst

$(BINDIR)/%: source/algos/%.c $(ALGOSINC)
	@test -d $(BINDIR) || mkdir $(BINDIR)
	$(CC) $(CFLAGS) $< -o $@
./%-fuzz: source/%.c $(SRCINC) source/algos/include/shmids.h
	$(CC) $(CFLAGS) -DFUZZ $< -std=gnu99 -o $@ -lm
./%-asan: source/%.c $(SRCINC) source/algos/include/shmids.h
	$(CC) $(CFLAGS) $< -std=gnu99 -o $@ -lm
./%: source/%.c $(SRCINC) source/algos/include/shmids.h
	$(CC) $(CFLAGS) $< -std=gnu99 -o $@ -lm
$(SELECTBIN): source/selectAlgo.c $(SRCINC)
	$(CC) $(CFLAGS) $< -o $@
verify/%.vfy: source/algos/%.c $(ALGOSINC)
	@$(MAKE) -s algocfg
	b=`basename $@ .vfy`; ./algocfg $$b cbmc; args=`./algocfg $$b cbmc`; \
	  echo $(TIMEOUT_4m) cbmc $$args $(CBMC_ARGS) $(CBMC_CHECKS) $<; \
	  $(TIMEOUT_4m) cbmc $$args $(CBMC_ARGS) $(CBMC_CHECKS) $< | tee $@
verify/%.vfy-trace: source/algos/%.c $(ALGOSINC)
	@$(MAKE) -s algocfg
	b=`basename $@ .vfy-trace`; ./algocfg $$b cbmc; args=`./algocfg $$b cbmc`; \
	  echo $(TIMEOUT_4m) cbmc --trace $$args $(CBMC_ARGS) $(CBMC_CHECKS) $<; \
	  $(TIMEOUT_4m) cbmc --trace $$args $(CBMC_ARGS) $(CBMC_CHECKS) $< | tee $@

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
	for t in $(TESTS); do echo $$t; $(DRV) ./$(TESTBIN) $$t; done
	for t in $(TESTS); do echo $$t rand2 2; $(DRV) ./$(TESTBIN) $$t rand2 2; done
	$(DRV) ./$(SELECTBIN) -all block bmh2 bmh4 dfdm sbdm faoso2 blim ssecp
	-mv algorithms.lst.bak algorithms.lst
lint: cppcheck clang-tidy sanitizer.log
# for newer cppcheck
#CPPCHECK_ARGS="-j4 --enable=warning,portability --inline-suppr --check-level=exhaustive"
CPPCHECK_ARGS = -j4 --enable=warning,portability --inline-suppr
cppcheck:
	cppcheck $(CPPCHECK_ARGS) source/*.c source/algos/*.c
compile_commands.json: GNUmakefile
	-+$(MAKE) clean
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
	./algocfg good 0 | perl -nle'%bad=map{$$_=>1}split/ /;for(split/ /,qx"./algocfg asan"){print $$_ unless $$bad{$$_}}' >$@

# MAX_M 10 * MAX_N 36
CBMC_ARGS = -DCBMC --slice-formula
CBMC_CHECKS=--bounds-check --pointer-check --memory-leak-check            \
  --div-by-zero-check --signed-overflow-check --unsigned-overflow-check   \
  --pointer-overflow-check --conversion-check --undefined-shift-check     \
  --float-overflow-check --nan-check --enum-range-check
  # cbmc 5.12.1: --pointer-primitive-check
# UNSATISFIABLE: passes, but needs more depth or builtins (nested loops => memset)
UNSATISFIABLE  = bf ac smith br akc bfs graspm ssef skip5 skip6 skip7 skip8 bndml \
	bmh-sbndm aoso2 aoso4 aoso6 blim bndmq2 bndmq4 bndmq6 bsdm bsdm6 bsdm8 fsbndm-w8 ssm 
FAIL_VERIFY    = smoa fs ssabs hash3 hash5 hash8 so sbndm svm0 svm3 svm4 bww faoso2 faoso4 ufndmq4 \
	ufndmq6 ufndmq8 ksa kbndm bsdm2 fs-w4 fs-w6 ssecp libc libc1 musl simdkr
FAIL = gs gg rcolussi bmh2 bmh4 graspm simon ldm sbdm bsom bmh-sbndm faoso4 faoso6 blim ksa \
	bsdm4 bxs fs-w2 fs-w4 fsbndm-w2 fsbndm-w4 fsbndm-w6 fsbndmq32 fsbndmq42 fsbndmq43 fsbndmq62 \
	fsbndmq64 fsbndmq82 fsbndmq84 fsbndmq86 qf26 sbndm-w2 sbndm-w4 tsa tsa-q2 tvsbs-w4 tvsbs-w6 \
	tvsbs-w8 hpbm ssecp libc libc1 simdkr
TIMEOUT_VERIFY = bm gs ag colussi gg skip askip ffs aut simon fdm bom bom2 dfdm ww ldm ildm1 ildm2 ebom \	fbom sebom sfbom skip2 skip3 skip4 bndm tndm lbndm dbww dbww2 bsdm3 bsdm4 bsdm5 bsdm7 bxs \
	fsbndmq20 fsbndmq21 fsbndmq31 fsbndmq32 fsbndmq41 fsbndmq42 fsbndmq43 fsbndmq61 fsbndmq62 \
	fsbndmq64 fsbndmq81 fsbndmq82 fsbndmq84 fsbndmq86 tsa tsa-q2 tso5 epsm
NON_CBMC_SRC   = $(addsuffix .c, $(addprefix source/algos/,$(TIMEOUT_VERIFY) $(FAIL_VERIFY)))
verify: verify/verify.log
verify/verify.log: $(filter-out $(NON_CBMC_SRC),$(ALGOSRC)) algocfg
	for c in $(filter-out $(NON_CBMC_SRC),$(ALGOSRC)); do \
	  echo $$c; b=`basename $$c .c`; ./algocfg $$b cbmc; args=`./algocfg $$b cbmc`; \
	  echo $(TIMEOUT_4m) cbmc $$args $(CBMC_ARGS) $(CBMC_CHECKS) $$c; \
	  $(TIMEOUT_4m) cbmc $$args $(CBMC_ARGS) $(CBMC_CHECKS) $$c || \
            (echo cbmc $$args $(CBMC_ARGS) $(CBMC_CHECKS) "$$c FAILED"; \
	    test $(( `./algocfg $$b VFY_FAIL` + `./algocfg $$b VFY_TIMEOUT` )) -gt 0 || exit 1); \
	done | tee verify/verify.log
check-verify:
	for c in $(addsuffix .c, $(addprefix source/algos/,$(filter-out $(TIMEOUT_VERIFY),$(TESTS)))); \
	do \
	  echo $$c; b=`basename $$c .c`; args=`./algocfg $$b cbmc`; \
	  echo $(TIMEOUT_1m) cbmc $$args $(CBMC_ARGS) $(CBMC_CHECKS) $$c; \
	  $(TIMEOUT_1m) cbmc $$args $(CBMC_ARGS) $(CBMC_CHECKS) $$c || \
            (echo cbmc $(CBMC_ARGS_0) $(CBMC_CHECKS) "$$c FAILED"; \
	     test $(( `./algocfg $$b VFY_FAIL` + `./algocfg $$b VFY_TIMEOUT` )) -gt 0 || exit 1); \
	done
# prints the violations
verify-trace: verify/trace.log
verify/trace.log: $(addsuffix .c, $(addprefix source/algos/,$(FAIL_VERIFY)))
	for c in $(addsuffix .c, $(addprefix source/algos/,$(FAIL_VERIFY))); do \
	  echo $$c; b=`basename $$c .c`; args=`./algocfg $$b cbmc`; \
	  echo $(TIMEOUT_4m) cbmc --trace $$args $(CBMC_ARGS) $(CBMC_CHECKS) $$c; \
	  $(TIMEOUT_4m) cbmc --trace $$args $(CBMC_ARGS) $(CBMC_CHECKS) $$c || \
            (echo cbmc --trace $$args $(CBMC_ARGS) $(CBMC_CHECKS) " $$c FAILED"; \
	     test $(( `./algocfg $$b VFY_FAIL` + `./algocfg $$b VFY_TIMEOUT` )) -gt 0 || exit 1); \
	done | tee verify/trace.log
fuzz: test-fuzz
	for c in $(ALGOSRC); do \
	  b="`basename $$c .c`"; \
	  $(MAKE) FUZZ=1 bin/fuzz/$$b; \
	  $(TIMEOUT_1m) afl-fuzz -i data/midimusic -o fuzz/$$b -- bin/fuzz/$$b; \
	done

fmt:
	clang-format -i `find source -name \*.c -o -name \*.h`
clean:
	-rm -f $(BINS) $(HELPERS) 2>/dev/null
TAGS: $(ALLSRC)
	-rm -f TAGS 2>/dev/null
	find . \( -name \*.c -o -name \*.h \) -exec etags -a --language=c \{\} \;
