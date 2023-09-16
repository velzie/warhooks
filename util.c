#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

// Function to find the base address of a module in a process
uintptr_t findBaseAddress(int pid, const char *moduleName,
                          const char *pagetype) {
  uintptr_t baseAddress = 0;
  char mapsFilePath[1024];

  // Create the path to the maps file for the process
  snprintf(mapsFilePath, sizeof(mapsFilePath), "/proc/%d/maps", pid);

  // Open the maps file
  FILE *mapsFile = fopen(mapsFilePath, "r");
  if (!mapsFile) {
    perror("Error opening maps file");
    return 0;
  }

  // Iterate through the lines in the maps file
  char line[1024];
  while (fgets(line, sizeof(line), mapsFile)) {
    // find the executable page
    if (strstr(line, pagetype)) {
      // Check if the line contains the module name
      if (strstr(line, moduleName)) {
        // Parse the start address
        sscanf(line, "%lx", &baseAddress);
        break;
      }
    }
  }

  // Close the maps file
  fclose(mapsFile);

  return baseAddress;
}

void RemoveChars(char *s, char c) {
  int writer = 0, reader = 0;

  while (s[reader]) {
    if (s[reader] != c) {
      s[writer++] = s[reader];
    }

    reader++;
  }

  s[writer] = 0;
}
