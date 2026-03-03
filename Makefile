CXX         := g++
LLVM_CONFIG := /usr/lib/llvm-18/bin/llvm-config
LLVM_INCDIR := $(shell $(LLVM_CONFIG) --includedir)
LLVM_DEFNS  := -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS
LLVM_LDFLAGS  := $(shell $(LLVM_CONFIG) --ldflags)
LLVM_LIBS     := $(shell $(LLVM_CONFIG) --libs core support)

CXXFLAGS := -std=c++17 -I$(LLVM_INCDIR) $(LLVM_DEFNS) -Isrc -Wall -Wno-unused-variable -Wno-unused-function -Wno-overloaded-virtual -Wno-pessimizing-move
LDFLAGS  := $(LLVM_LDFLAGS) $(LLVM_LIBS) -lpthread -ldl -lm

SRCDIR := src
BUILDDIR := build

SRCS := $(wildcard $(SRCDIR)/*.cpp)
OBJS := $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(SRCS))

TARGET := brandy

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

clean:
	rm -rf $(BUILDDIR) $(TARGET)
