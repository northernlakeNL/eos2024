#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

std::string translate(std::string line, std::string argument) {
  std::string result = ""; // implementeer dit
  std::vector<char> vowels{'A', 'E', 'I', 'O', 'U'};
  char dash = '-';
  std::ostringstream temp;
  std::ostringstream new_line;
  int remove_counter = 0;

  for (char &c : line) {
    char Uc = std::toupper(c);
    if (std::find(vowels.begin(), vowels.end(), Uc) != vowels.end()) {
      break;
    } else {
      temp << c;
      remove_counter++;
    }
  }
  line.erase(0,remove_counter);
  new_line << line << dash << temp.str() << argument;
  result = new_line.str();
  return result;
}

int main(int argc, char *argv[]) {
  std::string line;

  if (argc != 2) {
    std::cerr << "Deze functie heeft exact 1 argument nodig" << std::endl;
    return -1;
  }
  while (std::getline(std::cin, line)) {
    std::cout << translate(line, argv[1]) << std::endl;
  }
  return 0;
}
