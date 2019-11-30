#include <elf.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define DUMP(x)                                                                \
  do {                                                                         \
    printf("  " #x " = %u(0x%x)\n", (uint32_t)x, (uint32_t)x);                 \
  } while (0);

void dump_ehdr(Elf64_Ehdr *ehdr) {
  int i;
  printf("  ehdr->e_ident = ");
  for (i = 0; i < EI_NIDENT; i++) {
    printf("%02x ", ehdr->e_ident[i]);
  }
  printf("\n");
  DUMP(ehdr->e_type);
  DUMP(ehdr->e_machine);
  DUMP(ehdr->e_version);
  DUMP(ehdr->e_entry);
  DUMP(ehdr->e_phoff);
  DUMP(ehdr->e_shoff);
  DUMP(ehdr->e_flags);
  DUMP(ehdr->e_ehsize);
  DUMP(ehdr->e_phentsize);
  DUMP(ehdr->e_phnum);
  DUMP(ehdr->e_shentsize);
  DUMP(ehdr->e_shnum);
  DUMP(ehdr->e_shstrndx);
  printf("\n");
}

void dump_phdr(Elf64_Phdr *phdr, int e_phnum) {
  for (int i = 0; i < e_phnum; i++, phdr++) {
    printf("  index %d\n", i);
    DUMP(phdr->p_type);
    DUMP(phdr->p_flags);
    DUMP(phdr->p_offset);
    DUMP(phdr->p_vaddr);
    DUMP(phdr->p_filesz);
    DUMP(phdr->p_memsz);
    DUMP(phdr->p_align);
    printf("\n");
  }
  printf("\n");
}

void dump_stringtbl(unsigned char *str, Elf64_Shdr *shdr) {
  unsigned char *tbl_head = &str[shdr->sh_offset];
  unsigned long total_len = 0;

  while (total_len < shdr->sh_size) {
    printf("  %03lu: %s\n", (&tbl_head[total_len] - tbl_head),
           &tbl_head[total_len]);
    total_len += strlen((char *)&tbl_head[total_len]) + 1;
  }
  printf("\n");
}

void dump_shdr(Elf64_Ehdr *ehdr, Elf64_Shdr *shdr, int e_shnum) {
  uint32_t strtbl_offset = shdr[ehdr->e_shstrndx].sh_offset;
  char *buf = (char *)ehdr;
  char *strtbl = &buf[strtbl_offset];
  for (int i = 0; i < e_shnum; i++, shdr++) {
    char *name = &strtbl[shdr->sh_name];
    printf("  index %d: %s\n", i, name);
    DUMP(shdr->sh_name);
    DUMP(shdr->sh_type);
    DUMP(shdr->sh_flags);
    DUMP(shdr->sh_addr);
    DUMP(shdr->sh_offset);
    DUMP(shdr->sh_size);
    DUMP(shdr->sh_link);
    DUMP(shdr->sh_info);
    DUMP(shdr->sh_addralign);
    DUMP(shdr->sh_entsize);
    printf("\n");
  }
  printf("\n");
}

void usage() { printf("usage: relf [option] [file]\n"); }

int main(int argc, char *argv[]) {
  char mode;
  char *filename;
  const char *optstring = "hpsS";
  int c;
  while ((c = getopt(argc, argv, optstring)) != -1) {
    switch (c) {
    case 'h':
    case 'p':
    case 's':
    case 'S':
      mode = (char)c;
      break;
    default:
      break;
    }
  }

  if (argc == optind) {
    usage();
    return EXIT_FAILURE;
  }

  filename = argv[optind];

  FILE *fp = fopen(filename, "r");
  Elf64_Ehdr *ehdr;
  Elf64_Phdr *phdr;
  Elf64_Shdr *shdr;
  struct stat st;
  stat(filename, &st);

  unsigned char buf[st.st_size];
  fread(buf, 1, st.st_size, fp);
  fclose(fp);

  ehdr = (Elf64_Ehdr *)buf;
  phdr = (Elf64_Phdr *)(&buf[ehdr->e_phoff]);
  shdr = (Elf64_Shdr *)(&buf[ehdr->e_shoff]);
  int sh_name;

  switch (mode) {
  case 'h':
    printf("Elf file header(equivalent as readelf -h)\n");
    dump_ehdr(ehdr);
    break;
  case 'p':
    printf("Program header(equivalent as readelf -l)\n");
    dump_phdr(phdr, ehdr->e_phnum);
    break;
  case 's':
    printf("Section header(equivalent as readelf -S)\n");
    dump_shdr(ehdr, shdr, ehdr->e_shnum);
    break;
  case 'S':
    printf("String table\n");
    dump_stringtbl(buf, &shdr[ehdr->e_shstrndx]);
    break;
  default:
    return EXIT_FAILURE;
  }

  return 0;
}
