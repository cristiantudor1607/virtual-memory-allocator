CC=gcc
CFLAGS=-Wall -Wextra -std=c99

TARGETS = vma

build: $(TARGETS)

vma: main.c vma.c funcs.c text_utils.c DLL.c
	$(CC) $(CFLAGS) main.c vma.c funcs.c text_utils.c DLL.c -o vma

run_vma: vma
	./vma

clean:
	rm -f $(TARGETS)