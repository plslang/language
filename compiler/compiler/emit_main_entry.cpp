// compiler/emit_main_entry.cpp - emit C++ int main() dispatcher
#include "compiler.hpp"

using namespace std;

string emitMainEntry(const map<string, FuncInfo>& funcs) {
    ostringstream out;

    out << "// real entry\n";
    out << "int main() {\n";

    auto it = funcs.find("main");
    const vector<DeclareInfo> empty;
    const vector<DeclareInfo>& mainDecls =
        (it != funcs.end()) ? it->second.declares : empty;

    for (const auto& d : mainDecls) {
        out << "    // owner for declare " << d.type << " " << d.name << "\n";
        out << "    " << typeFor(d.type) << " "
            << cppIdentFor("main", d.name) << " = "
            << defaultFor(d.type) << ";\n";
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