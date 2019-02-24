/**
  *  Worg
  */
#include <stdio.h>
#include <string.h>

#include "common_data.h"
#include "image_encryption.h"

int main() {
  char file_name[100];
  char encrypted_file_name[100];
  char decrypted_file_name[100];
  char secret_file_name[100];

  //  Subtask (1)

  printf("Dati numele fisierului in care se afla imaginea pe care o doriti incriptata: ");
  fgets(file_name, 101, stdin);
  file_name[strlen(file_name) - 1] = '\0';

  printf("Dati numele fisierului in care se va afla imaginea criptata: ");
  fgets(encrypted_file_name, 101, stdin);
  encrypted_file_name[strlen(encrypted_file_name) - 1] = '\0';

  printf("...si, daca se poate, dati si numele fisierului in care se afla cheile necesare criptarii: ");
  fgets(secret_file_name, 101, stdin);
  secret_file_name[strlen(secret_file_name) - 1] = '\0';
  EncryptImage(file_name, encrypted_file_name, secret_file_name);

  //  Subtask (2)

  printf("Dati numele fisierului in care se afla imaginea criptata: ");
  fgets(encrypted_file_name, 101, stdin);
  encrypted_file_name[strlen(encrypted_file_name) - 1] = '\0';

  printf("Dati numele fisierului in care vreti sa fie pusa imaginea decriptata: ");
  fgets(decrypted_file_name, 101, stdin);
  decrypted_file_name[strlen(decrypted_file_name) - 1] = '\0';

  printf("Din nou, numele fisierului in care se afla cheile: ");
  fgets(secret_file_name, 101, stdin);
  secret_file_name[strlen(secret_file_name) - 1] = '\0';

  DecryptImage(encrypted_file_name, decrypted_file_name, secret_file_name);

  //  Subtask (3)

  printf("Testul chi_patrat pentru imaginea initiala:\n");
  PrintBMPTest(file_name);
  printf("Testul chi_patrat pentru imaginea criptata:\n");
  PrintBMPTest(encrypted_file_name);

  return 0;
}