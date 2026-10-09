// compiler/emit_main_entry.cpp - emit C++ int main() dispatcher
#include <string>
#include <sstream>
#include <vector>
#include <map>

using namespace std;

struct DeclareInfo {
    string type;
    string name;
};

string cppIdentFor(const string& fn, const string& var);

string emitMainEntry(const map<string, vector<DeclareInfo>>& declares) {
    ostringstream out;

    out << "// real entry\n";
    out << "int main() {\n";

    auto it = declares.find("main");
    const vector<DeclareInfo> empty;
    const vector<DeclareInfo>& mainDecls = (it != declares.end()) ? it->second : empty;

    for (const auto& d : mainDecls) {
        out << "    // owner for declare " << d.type << " " << d.name << "\n";
        if (d.type == "int") {
            out << "    long long " << cppIdentFor("main", d.name) << " = 0;\n";
        } else if (d.type == "string") {
            out << "    std::string " << cppIdentFor("main", d.name) << " = \"\";\n";
        } else {
            out << "    auto " << cppIdentFor("main", d.name) << " = 0;\n";
        }
    }

    out << "    fn_main(";
    for (size_t i = 0; i < mainDecls.size(); ++i) {
        if (i) out << ", ";
        out << cppIdentFor("main", mainDecls[i].name);
    }
    out << ");\n";

    out << "    return 0;\n";
    out << "}\n";

    return out.str();
}