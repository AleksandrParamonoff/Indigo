﻿/****************************************************************************
 * Copyright (C) from 2009 to Present EPAM Systems.
 *
 * This file is part of Indigo toolkit.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

#include <functional>
#include <rapidjson/document.h>
#include <string>
#include <unordered_map>
#include <vector>

#include "common/base_cpp/scanner.h"
#include "layout/molecule_layout.h"
#include "molecule/elements.h"
#include "molecule/molecule.h"
#include "molecule/molecule_json_loader.h"
#include "molecule/monomer_commons.h"
#include "molecule/monomers_basic_templates.h"
#include "molecule/monomers_lib.h"
#include "molecule/smiles_loader.h"

namespace indigo
{
    IMPL_ERROR(MonomerTemplates, "monomers library");

    struct MonomerDescriptor
    {
        MonomerClass monomer_type;
        std::string template_id;
        std::vector<std::string> aliases;
        std::string natreplace;
    };

    struct NucleotideDescriptor
    {
        NucleotideType nucleo_type;
        std::vector<std::string> aliases;
        std::unordered_map<MonomerClass, std::string> components;
    };

    const MonomerTemplates& MonomerTemplates::_instance()
    {
        static MonomerTemplates instance;
        return instance;
    }

    const std::string& MonomerTemplates::classToStr(MonomerClass mon_type)
    {
        static const std::unordered_map<MonomerClass, std::string> kMonomerTypeStr = {{MonomerClass::Phosphate, kMonomerClassPHOSPHATE},
                                                                                      {MonomerClass::Sugar, kMonomerClassSUGAR},
                                                                                      {MonomerClass::Base, kMonomerClassBASE},
                                                                                      {MonomerClass::AminoAcid, kMonomerClassAA},
                                                                                      {MonomerClass::CHEM, kMonomerClassCHEM}};

        return kMonomerTypeStr.at(mon_type);
    }

    bool MonomerTemplates::splitNucleotide(std::string nucleo_type, std::string alias, GranularNucleotide& splitted_nucleotide)
    {
        NucleotideType nt = NucleotideType::RNA;
        if (isDNAClass(nucleo_type))
            nt = NucleotideType::DNA;
        else if (!isRNAClass(nucleo_type))
            return false;
        return splitNucleotide(nt, alias, splitted_nucleotide);
    }

    bool MonomerTemplates::splitNucleotide(NucleotideType nucleo_type, std::string alias, GranularNucleotide& splitted_nucleotide)
    {
        auto it = _instance()._nucleotides_lib.find(std::make_pair(nucleo_type, alias));
        if (it != _instance()._nucleotides_lib.end())
        {
            splitted_nucleotide = it->second;
            return true;
        }
        return false;
    }

    const std::unordered_map<std::string, MonomerClass>& MonomerTemplates::getStrToMonomerType()
    {
        static const std::unordered_map<std::string, MonomerClass> kStrMonomerType = {{kMonomerClassSUGAR, MonomerClass::Sugar},
                                                                                      {kMonomerClassPHOSPHATE, MonomerClass::Phosphate},
                                                                                      {kMonomerClassBASE, MonomerClass::Base},
                                                                                      {kMonomerClassAA, MonomerClass::AminoAcid},
                                                                                      {kMonomerClassCHEM, MonomerClass::CHEM}};
        return kStrMonomerType;
    }

    bool MonomerTemplates::getMonomerTemplate(MonomerClass mon_type, std::string alias, TGroup& tgroup)
    {
        auto it = _instance()._monomers_lib.find(std::make_pair(mon_type, alias));
        if (it != _instance()._monomers_lib.end())
        {
            tgroup.copy(it->second.get());
            return true;
        }
        return false;
    }

    bool MonomerTemplates::getMonomerTemplate(std::string mon_type, std::string alias, TGroup& tgroup)
    {
        auto ct_it = getStrToMonomerType().find(mon_type);
        if (ct_it != _instance().getStrToMonomerType().end())
            return getMonomerTemplate(ct_it->second, alias, tgroup);
        return false;
    }

    MonomerTemplates::MonomerTemplates() : _templates_mol(new Molecule())
    {
        initializeMonomers();
    }

    void MonomerTemplates::initializeMonomers()
    {
        static const std::vector<NucleotideDescriptor> nucleotide_presets = {
            {NucleotideType::RNA, {"A", "AMP"}, {{MonomerClass::Base, "A"}, {MonomerClass::Sugar, "R"}, {MonomerClass::Phosphate, "P"}}},
            {NucleotideType::RNA, {"C", "CMP"}, {{MonomerClass::Base, "C"}, {MonomerClass::Sugar, "R"}, {MonomerClass::Phosphate, "P"}}},
            {NucleotideType::RNA, {"G", "GMP"}, {{MonomerClass::Base, "G"}, {MonomerClass::Sugar, "R"}, {MonomerClass::Phosphate, "P"}}},
            {NucleotideType::RNA, {"T", "TMP"}, {{MonomerClass::Base, "T"}, {MonomerClass::Sugar, "R"}, {MonomerClass::Phosphate, "P"}}},
            {NucleotideType::RNA, {"U", "UMP"}, {{MonomerClass::Base, "U"}, {MonomerClass::Sugar, "R"}, {MonomerClass::Phosphate, "P"}}},
            {NucleotideType::DNA, {"A", "dA", "dAMP"}, {{MonomerClass::Base, "A"}, {MonomerClass::Sugar, "dR"}, {MonomerClass::Phosphate, "P"}}},
            {NucleotideType::DNA, {"C", "dC", "dCMP"}, {{MonomerClass::Base, "C"}, {MonomerClass::Sugar, "dR"}, {MonomerClass::Phosphate, "P"}}},
            {NucleotideType::DNA, {"G", "dG", "dGMP"}, {{MonomerClass::Base, "G"}, {MonomerClass::Sugar, "dR"}, {MonomerClass::Phosphate, "P"}}},
            {NucleotideType::DNA, {"T", "dT", "dTMP"}, {{MonomerClass::Base, "T"}, {MonomerClass::Sugar, "dR"}, {MonomerClass::Phosphate, "P"}}},
            {NucleotideType::DNA, {"U", "dU", "dUMP"}, {{MonomerClass::Base, "U"}, {MonomerClass::Sugar, "dR"}, {MonomerClass::Phosphate, "P"}}}};

        // collect basic monomers
        using namespace rapidjson;
        Document data;
        if (!data.Parse(kMonomersBasicTemplates).HasParseError())
        {
            MoleculeJsonLoader loader(data);
            loader.stereochemistry_options.ignore_errors = true;
            loader.loadMolecule(_templates_mol->asMolecule());
            for (auto i = _templates_mol->tgroups.begin(); i != _templates_mol->tgroups.end(); i = _templates_mol->tgroups.next(i))
            {
                auto& tg = _templates_mol->tgroups.getTGroup(i);

                _monomers_lib.emplace(MonomerKey(getStrToMonomerType().at(tg.tgroup_class.ptr()), tg.tgroup_alias.ptr()), std::ref(tg));
            }
        }

        // create nucleotides' mappings
        for (const auto& desc : nucleotide_presets)
        {
            std::unordered_map<MonomerClass, std::reference_wrapper<MonomersLib::value_type>> nucleotide_triplet;
            // iterate nucleotide components
            for (auto& kvp : desc.components)
            {
                // find nucleotide component
                auto comp_it = _monomers_lib.find(std::make_pair(kvp.first, kvp.second));
                if (comp_it != _monomers_lib.end())
                    nucleotide_triplet.emplace(kvp.first, std::ref(*comp_it));
            }

            for (const auto& alias : desc.aliases)
                _nucleotides_lib.emplace(std::make_pair(desc.nucleo_type, alias), nucleotide_triplet);
        }
    }

    IMPL_ERROR(IdtAlias, "IDT alias");

    const std::string& IdtAlias::getModification(IdtModification modification)
    {
        static std::string empty;
        switch (modification)
        {
        case IdtModification::FIVE_PRIME_END:
            return getFivePrimeEnd();
        case IdtModification::INTERNAL:
            return getInternal();
        case IdtModification::THREE_PRIME_END:
            return getThreePrimeEnd();
        };
        throw Error("Unknown IDT modification: %s.", modification);
        return empty;
    }

    const std::string& IdtAlias::getFivePrimeEnd()
    {
        if (_five_prime_end != "")
            return _five_prime_end;
        else
            throw Error("IDT alias %s has no five-prime end modification.", _base.c_str());
    }

    const std::string& IdtAlias::getInternal()
    {
        if (_internal != "")
            return _internal;
        else
            throw Error("IDT alias %s has no internal modification.", _base.c_str());
    }

    const std::string& IdtAlias::getThreePrimeEnd()
    {
        if (_three_prime_end != "")
            return _three_prime_end;
        else
            throw Error("IDT alias %s has no three-prime end modification.", _base.c_str());
    }

    IMPL_ERROR(MonomerTemplate, "MonomerTemplate");

    const std::string& MonomerTemplate::MonomerClassToStr(MonomerClass monomer_type)
    {
        static const std::map<MonomerClass, std::string> _type_to_str{
            {MonomerClass::AminoAcid, "AminoAcid"},   {MonomerClass::Sugar, "Sugar"},   {MonomerClass::Phosphate, "Phosphate"}, {MonomerClass::Base, "Base"},
            {MonomerClass::Terminator, "Terminator"}, {MonomerClass::Linker, "Linker"}, {MonomerClass::Unknown, "Unknown"},     {MonomerClass::CHEM, "Chem"}};

        return _type_to_str.at(monomer_type);
    }

    const MonomerClass MonomerTemplate::StrToMonomerClass(const std::string& monomer_type)
    {
        static const std::map<std::string, MonomerClass> _str_to_type = {
            {"AminoAcid", MonomerClass::AminoAcid},   {"Sugar", MonomerClass::Sugar},   {"Phosphate", MonomerClass::Phosphate}, {"Base", MonomerClass::Base},
            {"Terminator", MonomerClass::Terminator}, {"Linker", MonomerClass::Linker}, {"Unknown", MonomerClass::Unknown},     {"Chem", MonomerClass::CHEM}};
        if (_str_to_type.count(monomer_type))
            return _str_to_type.at(monomer_type);
        return MonomerClass::Unknown;
    }

    MonomerTemplate::MonomerTemplate(const std::string& id, const std::string& mt_class, const std::string& class_HELM, const std::string& full_name,
                                     const std::string& alias, const std::string& natural_analog, int tgroup_id, BaseMolecule& mol)
        : _id(id), _class(StrToMonomerClass(mt_class)), _class_HELM(class_HELM), _full_name(full_name), _alias(alias), _natural_analog(natural_analog),
          _tgroup_id(tgroup_id), _mol(mol)
    {
    }

    void MonomerTemplate::AddAttachmentPoint(const std::string& id, const std::string& ap_type, int att_atom, std::vector<int>& leaving_group)
    {
        std::string ap_id = id.size() != 0 ? id : "R" + std::to_string(1 + _attachment_points.size());
        AttachmentPointType attachment_point_type;
        if (ap_type == "left")
            attachment_point_type = AttachmentPointType::LEFT;
        else if (ap_type == "right")
            attachment_point_type = AttachmentPointType::RIGHT;
        else if (ap_type == "side")
            attachment_point_type = AttachmentPointType::SIDE;
        else if (_attachment_points.size() == 0) // first is left
            attachment_point_type = AttachmentPointType::LEFT;
        else if (_attachment_points.size() == 0) // second is right
            attachment_point_type = AttachmentPointType::RIGHT;
        else // all other is side
            attachment_point_type = AttachmentPointType::SIDE;
        AddAttachmentPoint(AttachmentPoint(ap_id, attachment_point_type, att_atom, leaving_group));
    }

    const AttachmentPoint& MonomerTemplate::getAttachmenPointById(const std::string& att_point_id)
    {
        if (!hasAttachmenPointWithId(att_point_id))
            throw Error("Attachment point with id %s not found in monomer template %s.", att_point_id.c_str(), _id.c_str());
        return _attachment_points.at(att_point_id);
    }

    const TGroup& MonomerTemplate::getTGroup() const
    {
        return _mol.tgroups.getTGroup(_tgroup_id);
    }

    IMPL_ERROR(MonomerGroupTemplate, "MonomerGroupTemplate");

    void MonomerGroupTemplate::addTemplate(const std::string& template_id)
    {
        _monomer_templates.insert(
            std::pair<std::string, MonomerTemplate&>(template_id, MonomerTemplateLibrary::instance().getMonomerTemplateById(template_id)));
    };

    MonomerTemplate& MonomerGroupTemplate::getTemplateByClass(MonomerClass monomer_class)
    {
        for (auto& id_template : _monomer_templates)
        {
            if (id_template.second.monomerClass() == monomer_class)
                return id_template.second;
        }
        throw Error("Monomer template with class %s not found in monomer temmplate group %s.", MonomerTemplate::MonomerClassToStr(monomer_class).c_str(),
                    _id.c_str());
    }

    IMPL_ERROR(MonomerTemplateLibrary, "MonomerTemplateLibrary");

    MonomerTemplateLibrary& MonomerTemplateLibrary::instance()
    {
        static MonomerTemplateLibrary library_instance;
        return library_instance;
    }

    MonomerTemplate& MonomerTemplateLibrary::getMonomerTemplateById(const std::string& monomer_template_id)
    {
        if (_monomer_templates.count(monomer_template_id) == 0)
            throw Error("Monomert template with id %s not found.", monomer_template_id.c_str());
        return _monomer_templates.at(monomer_template_id);
    }

    MonomerGroupTemplate& MonomerTemplateLibrary::getMonomerGroupTemplateById(const std::string& monomer_group_template_id)
    {
        if (_monomer_group_templates.count(monomer_group_template_id) == 0)
            throw Error("Monomert group template with id %s not found.", monomer_group_template_id.c_str());
        return _monomer_group_templates.at(monomer_group_template_id);
    }

    std::string MonomerTemplateLibrary::getMonomerTemplateIdByIdtAliasAndMod(const std::string& alias, IdtModification mod)
    {
        for (auto& monomer_template : _monomer_templates)
        {
            if (monomer_template.second.hasIdtAlias(alias, mod))
                return monomer_template.first;
        };
        return "";
    };

    std::string MonomerTemplateLibrary::getMGTidByIdtAliasAndMod(const std::string& alias, IdtModification mod)
    {
        for (auto& mgt : _monomer_group_templates)
        {
            if (mgt.second.hasIdtAlias(alias, mod))
                return mgt.first;
        };
        return "";
    };
}
