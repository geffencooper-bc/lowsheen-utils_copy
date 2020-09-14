
#include "manifest.h"
#include <string>
#include <tinyxml2.h>
#include <iostream>

namespace lowsheen
{

bool Manifest::read(const char * filename)
{
    tinyxml2::XMLDocument xmlDoc;

    if(filename == nullptr)
    {
        return false;
    }

    if(filename[0] == '0')
    {
        return false;
    }

    int eResult = (int)xmlDoc.LoadFile(filename);
    if(eResult != 0)
    {
        return false;
    }

    tinyxml2::XMLElement* pRoot = xmlDoc.FirstChildElement("manifest");

    if(pRoot == nullptr)
    {
        return false;
    }

    machines.clear();
    paths.clear();

    tinyxml2::XMLElement* m = pRoot->FirstChildElement("machine");

    while(m != nullptr)
    {
        MachineEntry machine;
        int id;
        
        id = m->IntAttribute("id");

        machine.machine_name = std::string(m->Attribute("name"));
        tinyxml2::XMLElement* c = m->FirstChildElement("controller");

        if(c != nullptr)
        {
            machine.controller.controller_type = std::string(c->Attribute("type"));
            tinyxml2::XMLElement* p = c->FirstChildElement("program");

            while(p != nullptr)
            {                
                int id = p->IntAttribute("id");
                machine.controller.programs[id] = std::string(p->Attribute("filename"));
                p = p->NextSiblingElement("program");
            }

            p = c->FirstChildElement("params");
            while(p != nullptr)
            {
                int id = p->IntAttribute("id");
                machine.controller.params[id] = std::string(p->Attribute("filename"));
                p = p->NextSiblingElement("params");
            }
        }

        machines[id] = machine;

        m = m->NextSiblingElement("machine");

    }

    paths.clear();
    tinyxml2::XMLElement* f = pRoot->FirstChildElement("files");

    if(f != nullptr)
    {
        tinyxml2::XMLElement* p = f->FirstChildElement("path");
        while(p != nullptr)
        {
            std::string str = p->GetText();
            paths.push_back(str);
            p = p->NextSiblingElement("path");
        }
    }

    return true;
}

bool Manifest::write(const char * filename)
{
    tinyxml2::XMLDocument xmlDoc;
    tinyxml2::XMLNode * pRoot = xmlDoc.NewElement("manifest");
    xmlDoc.InsertFirstChild(pRoot);

    for (auto const& m : machines)
    {
        tinyxml2::XMLElement * pElement = xmlDoc.NewElement("machine");
        pElement->SetAttribute("id", m.first);
        pElement->SetAttribute("name", m.second.machine_name.c_str());

        tinyxml2::XMLElement * pElementController = xmlDoc.NewElement("controller");
        pElementController->SetAttribute("type", m.second.controller.controller_type.c_str());
                
        for (auto const& p : m.second.controller.programs)
        {
            tinyxml2::XMLElement * pElementProgram = xmlDoc.NewElement("program");
            pElementProgram->SetAttribute("id", p.first);
            pElementProgram->SetAttribute("filename", p.second.c_str());
            pElementController->InsertEndChild(pElementProgram);
        }

        for (auto const& p : m.second.controller.params)
        {
            tinyxml2::XMLElement * pElementParam = xmlDoc.NewElement("params");
            pElementParam->SetAttribute("id", p.first);
            pElementParam->SetAttribute("filename", p.second.c_str());
            pElementController->InsertEndChild(pElementParam);
        }   

        pElement->InsertEndChild(pElementController);

        pRoot->InsertEndChild(pElement);
    }



    tinyxml2::XMLElement * pElementPaths = xmlDoc.NewElement("files");    
    for (auto const& p : paths)
    {
        tinyxml2::XMLElement * pElementPath = xmlDoc.NewElement("path");
        tinyxml2::XMLText * pText = xmlDoc.NewText(p.c_str());
        pElementPath->InsertEndChild(pText);
        pElementPaths->InsertEndChild(pElementPath);
    }
    pRoot->InsertEndChild(pElementPaths);

    int eResult = (int)xmlDoc.SaveFile(filename);

    return eResult == 0;
}

bool Manifest::find(int *id, const char *id_or_name)
{
    for (auto const& m : machines)
    {
        if(std::to_string(m.first) == std::string(id_or_name))
        {
            *id = m.first;
            return true;
        }
        else if(m.second.machine_name == std::string(id_or_name))
        {
            *id = m.first;
            return true;
        }
    }

    return false;
}

bool Manifest::find(int id)
{
    auto it = machines.find(id);

    return it != machines.end();
}

}