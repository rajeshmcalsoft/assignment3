#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <cctype>

struct Host {
    std::string ip;
    std::string os;
    std::string memory;
    std::string cpu;
    std::string disk;
};

static double parseValue(const std::string& s) {
    std::stringstream ss;
    for (size_t i = 0; i < s.size(); i++) {
        if (std::isdigit(s[i]) || s[i] == '.') ss << s[i];
    }
    double val = 0;
    ss >> val;
    return val;
}

static std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n\"");
    size_t end = s.find_last_not_of(" \t\r\n\"");
    if (start == std::string::npos) return "";
    return s.substr(start, end - start + 1);
}

static std::string toLower(std::string s) {
    for (size_t i = 0; i < s.size(); i++) s[i] = std::tolower(s[i]);
    return s;
}

class Inventory {
    std::vector<Host> hosts;

public:
    void loadFromFile(const std::string& path) {
        std::ifstream file(path.c_str());
        if (!file.is_open())
            throw std::runtime_error("Cannot open file: " + path);

        std::string content((std::istreambuf_iterator<char>(file)),
                             std::istreambuf_iterator<char>());

        Host current;
        bool inHost = false;
        int braceDepth = 0;

        std::istringstream stream(content);
        std::string line;
        while (std::getline(stream, line)) {
            std::string trimmed = trim(line);
            if (trimmed.empty()) continue;

            if (trimmed.find("{") != std::string::npos) {
                braceDepth++;
                if (braceDepth == 3) {
                    inHost = true;
                    current = Host();
                }
            }

            if (inHost) {
                size_t colon = trimmed.find(':');
                if (colon != std::string::npos) {
                    std::string key = trim(trimmed.substr(0, colon));
                    std::string val = trim(trimmed.substr(colon + 1));
                    if (!val.empty() && val[val.size()-1] == ',') val.erase(val.size()-1);
                    val = trim(val);

                    if (key == "ip") current.ip = val;
                    else if (key == "os") current.os = val;
                    else if (key == "memory") current.memory = val;
                    else if (key == "cpu") current.cpu = val;
                    else if (key == "disk") current.disk = val;
                }
            }

            if (trimmed.find("}") != std::string::npos) {
                if (inHost && braceDepth == 3) {
                    hosts.push_back(current);
                    inHost = false;
                }
                braceDepth--;
            }
        }
    }

    void printHost(const Host& h) const {
        std::cout << "  IP: " << h.ip << "\n"
                  << "  OS: " << h.os << "\n"
                  << "  Memory: " << h.memory << "\n"
                  << "  CPU: " << h.cpu << "\n"
                  << "  Disk: " << h.disk << "\n\n";
    }

    void filter(const std::string& criteria) const {
        std::string c = toLower(criteria);

        if (c == "memory") {
            int maxIdx = 0;
            for (size_t i = 1; i < hosts.size(); i++) {
                if (parseValue(hosts[i].memory) > parseValue(hosts[maxIdx].memory))
                    maxIdx = i;
            }
            if (!hosts.empty()) printHost(hosts[maxIdx]);
        } else if (c == "cpu") {
            int maxIdx = 0;
            for (size_t i = 1; i < hosts.size(); i++) {
                if (parseValue(hosts[i].cpu) > parseValue(hosts[maxIdx].cpu))
                    maxIdx = i;
            }
            if (!hosts.empty()) printHost(hosts[maxIdx]);
        } else if (c == "linux" || c == "windows") {
            std::string target = (c == "linux") ? "Linux" : "Windows";
            for (size_t i = 0; i < hosts.size(); i++)
                if (hosts[i].os == target) printHost(hosts[i]);
        } else {
            throw std::invalid_argument("Invalid filter criteria: " + criteria +
                ". Valid: Memory, CPU, Linux, Windows");
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc < 3)
        throw std::runtime_error("Usage: InventoryFilter <json_file> <filter_criteria>");

    Inventory inv;
    inv.loadFromFile(argv[1]);
    inv.filter(argv[2]);
    return 0;
}
