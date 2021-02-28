#include <climits>
#include <fstream>
#include <iostream>
#include <map>
#include <string>

std::map<char, char> getMacronameMap() {
  std::map<char, char> map;
  for(char i = CHAR_MIN; i < CHAR_MAX; i++) {
    map[i] = '_';
  }
  for(char i = 'a'; i < 'z'; i++) {
    map[i] = i - 32;
  }
  for(char i = 'A'; i < 'Z'; i++) {
    map[i] = i;
  }
  return map;
}

std::map<char, std::string> getStringPrintMap() {
  std::map<char, std::string> map;
  char* buffer = new char[10];
  for(char i = CHAR_MIN; i < CHAR_MAX; i++) {
    switch(i) {
      case '\"':
        map[i] = "\\\"";
        break;
      case '\?':
        map[i] = "\\\?";
        break;
      case '\\':
        map[i] = "\\\\";
        break;
      case '\a':
        map[i] = "\\a";
        break;
      case '\b':
        map[i] = "\\b";
        break;
      case '\f':
        map[i] = "\\f";
        break;
      case '\n':
        map[i] = "\\n";
        break;
      case '\r':
        map[i] = "\\r";
        break;
      case '\t':
        map[i] = "\\t";
        break;
      case '\v':
        map[i] = "\\v";
        break;
      default:
        if(isprint(i)) {
          buffer[0] = i;
          buffer[1] = 0;
        } else {
          sprintf(buffer, "\"\\x%02X\"", reinterpret_cast<unsigned char&>(i));
        }
        map[i] = buffer;
    }
  }
  delete[] buffer;
  return map;
}

int main(int argc, char* argv[]) {
  if(argc != 3) {
    std::clog << "Usage: generator inputfile outputfile\n";
    return 1;
  }
  std::ifstream infile(argv[1], std::ios::in | std::ios::binary);
  std::ofstream outfile(argv[2], std::ios::out);

  auto macroMap = getMacronameMap();
  std::string macro;
  int pos = 0;
  while(char c = argv[2][pos++]) {
    macro.push_back(macroMap[c]);
  }
  if(macro.empty()) {
    std::clog << "Failed to generate macro\n";
    return 1;
  }

  std::string name;
  pos = 0;
  while(char c = std::tolower(argv[1][pos++])) {
    if(c >= 'a' && c <= 'z') {
      name.push_back(c);
    }
  }
  if(macro.empty()) {
    std::clog << "Failed to generate macro\n";
    return 1;
  }
  outfile << "#ifndef " << macro << '\n';
  outfile << "#define " << macro << '\n';
  outfile << "inline constexpr char " << name << "[] = " << '\n';

  auto printMap = getStringPrintMap();
  int count = 0;
  int newLineAfter = 120;
  char c = infile.get();
  bool inString = true;
  outfile << "\"";
  while(infile.good()) {
    const std::string& value = printMap[c];
    if(value[0] == '\"') {
      if(inString) {
        outfile << "\"";
        inString = false;
      }
    } else {
      if(!inString) {
        outfile << "\"";
        inString = true;
      }
    }
    outfile << value;

    count += value.size();
    infile.peek();
    if(!infile.good()) {
      outfile << (inString ? "\";\n" : ";\n");
    } else if(count > newLineAfter || c == '\n') {
      outfile << (inString ? "\"\n\"" : "\n");
      count = 0;
    }
    c = infile.get();
  }
  outfile << "#endif";
}
