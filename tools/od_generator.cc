#include "kickcat/Prints.h"
#include "kickcat/CoE/EsiParser.h"
#include <fstream>
#include <iostream>

namespace kickcat
{
    constexpr std::string_view OD_POPULATOR_FILE{"od_populator.cc"};

    void populateDictionnary(std::string esiFileName)
    {
        CoE::EsiParser parser;
        auto &coe_dict = CoE::dictionary();
        coe_dict = parser.load(esiFileName);
    }

    std::string addBeginning()
    {
        std::stringstream result;

        result << "/// This file is auto generated by od_generator.\n\n";

        result << "#include \"kickcat/CoE/OD.h\"\n\n";
        result << "namespace kickcat::CoE\n{\n";
        result << "    void populateOD()\n    {\n";

        return result.str();
    }

    std::string addEnding()
    {
        std::stringstream result;
        result << "    }\n}\n";
        return result.str();
    }



    std::string addEntry(CoE::Entry const &entryToAdd)
    {
        if (not CoE::isBasic(entryToAdd.type))
        {
            // TODO support complex type of entry
            THROW_ERROR("Object Dictionnary Generator only support basic type for now");
        }

        std::stringstream result;
        result << "            CoE::addEntry(object,";
        result << std::to_string(entryToAdd.subindex) << ",";
        result << std::to_string(entryToAdd.bitlen) << ",";
        result << std::to_string(entryToAdd.access) << ",";
        result << "static_cast<CoE::DataType>(" << std::to_string(static_cast<uint16_t>(entryToAdd.type)) << "),";
        result << "\"" << entryToAdd.description << "\",";
        result << dataToString(entryToAdd.type, entryToAdd.data);
        result << ");\n";

        return result.str();
    }

    std::string addObject(CoE::Object const &objectToAdd)
    {
        std::stringstream result;
        result << "        {\n";
        result << "            static CoE::Object object\n";
        result << "            {\n";
        result << "                .index = 0x" << std::hex << objectToAdd.index << std::dec << ",\n";
        result << "                .code = CoE::ObjectCode::" << CoE::toString(objectToAdd.code) << ",\n";
        result << "                .name = \"" << objectToAdd.name << "\",\n";
        result << "                .entries{}\n";
        result << "            };\n";

        for (auto const &entry : objectToAdd.entries)
        {
            result << addEntry(entry);
        }

        result << "            CoE::dictionary().push_back(std::move(object));\n";
        result << "        }\n\n";

        return result.str();
    }
}

int main(int argc, char *argv[])
{
    using namespace kickcat;

    if ((argc != 2))
    {
        printf("argc: %d\n", argc);
        printf("usage :    ./od_generator [esi.xml]\n");
        return 1;
    }

    populateDictionnary(argv[1]);

    std::ofstream f(std::string(OD_POPULATOR_FILE).c_str());

    /// Sort Dictionnary to have more readable source file
    std::sort(CoE::dictionary().begin(), CoE::dictionary().end(),
              [](CoE::Object &object1, CoE::Object &object2)
              { return object1.index < object2.index; });

    f << addBeginning();
    for (auto const &object : CoE::dictionary())
    {
        f << addObject(object);
    }
    f << addEnding();

    f.close();

    return 0;
}
