/****************************************************************************
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

#ifndef __metalayout_h__
#define __metalayout_h__

#include "base_cpp/obj_array.h"
#include "base_cpp/reusable_obj_array.h"
#include "math/algebra.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    class BaseMolecule;

    class DLLEXPORT Metalayout
    {
    public:
        struct DLLEXPORT LayoutItem
        {
            enum class ItemVerticalAlign
            {
                ECenter,
                ETop,
                EBottom
            };
            enum class Type
            {
                EMolecule = 0, ESpace = 1
            };

            LayoutItem()
            {
                clear();
            }

            void clear()
            {
                verticalAlign = ItemVerticalAlign::ECenter;
                type = Type::EMolecule;
                id = 0;
                isMoleculeFragment = false;
                min.zero();
                max.zero();
                scaledSize.zero();
                scaledOffset.zero();
                scaleFactor.zero();
                minScaledSize.zero();
            }
            Type type;
            int id;
            bool isMoleculeFragment;
            ItemVerticalAlign verticalAlign;

            Vec2f min, max;
            Vec2f scaledSize, scaledOffset, minScaledSize;
            Vec2f scaleFactor;
        };

        class DLLEXPORT LayoutLine
        {
        public:
            LayoutLine();
            ~LayoutLine();
            void clear();

            ObjArray<LayoutItem> items;
            float height;
            float top_height;
            float bottom_height;
            float width;

        private:
            LayoutLine(const LayoutLine&);
        };

        Metalayout();
        void clear();
        bool isEmpty() const;
        void prepare(); // calculates averageBondLength and scaleFactor
        float getAverageBondLength() const;
        float getScaleFactor() const;
        const Vec2f& getContentSize() const;
        void process();
        LayoutLine& newLine();
        static void getBoundRect(Vec2f& min, Vec2f& max, BaseMolecule& mol);
        void calcContentSize();
        void scaleMoleculesSize();

        void* context;
        void (*cb_process)(LayoutItem& item, const Vec2f& pos, void* context);
        BaseMolecule& (*cb_getMol)(int id, void* context);

        static float getTotalMoleculeBondLength(BaseMolecule& mol);
        static float getTotalMoleculeClosestDist(BaseMolecule& mol);

        // utility function to use in MoleculeLayout & ReactionLayout
        void adjustMol(BaseMolecule& mol, const Vec2f& min, const Vec2f& pos) const;

        float horizontalIntervalFactor;
        float verticalIntervalFactor;
        float bondLength;

        DECL_ERROR;

    private:
        Vec2f _contentSize;
        float _avel, _scaleFactor, _offset;

        float _getAverageBondLength();

        ReusableObjArray<LayoutLine> _layout;
    };
    
    struct UnitsOfMeasure
    {
        enum TYPE
        {
            PT,
            PX,
            INCHES,
            CM
        };

        static constexpr float INCH_TO_CM = 2.54f;
        static constexpr float PT_TO_PX = 1.333334f;

        static float convertToPx(const float input, const TYPE units, const float ppi)
        {
            switch (units)
            {
            case (PT):
                return input * PT_TO_PX;
                break;
            case (INCHES):
                return ppi * input;
                break;
            case (CM):
                return ppi * INCH_TO_CM * input;
                break;
            default:
                return input;
            }
        }

        static float convertToPt(const float input, const TYPE units, const float ppi)
        {

            switch (units)
            {
            case (PT):
                return input / PT_TO_PX;
                break;
            case (INCHES):
                return (input * ppi) / PT_TO_PX;
                break;
            case (CM):
                return (input * ppi * INCH_TO_CM) / PT_TO_PX;
                break;
            default:
                return input;
            }
        }

        static float convertToInches(const float input, const TYPE units, const float ppi)
        {
            switch (units)
            {
            case (PT):
                return (input * PT_TO_PX) / ppi;
                break;
            case (PX):
                return input / ppi;
                break;
            case (CM):
                return input * INCH_TO_CM;
                break;
            default:
                return input;
            }
        }

        static float convertToCm(const float input, const TYPE units, const float ppi)
        {
            switch (units)
            {
            case (PT):
                return (input * PT_TO_PX) / (ppi * INCH_TO_CM);
                break;
            case (INCHES):
                return input / INCH_TO_CM;
                break;
            case (PX):
                return input / (ppi * INCH_TO_CM);
                break;
            default:
                return input;
            }
        }
    };

    struct LayoutOptions
    {
        float bondLength{1.0f};
        UnitsOfMeasure::TYPE bondLengthUnit{UnitsOfMeasure::TYPE::PX};
        float reactionComponentMarginSize{0.5f};
        UnitsOfMeasure::TYPE reactionComponentMarginSizeUnit{UnitsOfMeasure::TYPE::PX};
        float ppi{72.0f};
        float getMarginSizeInAngstroms() const
        {
            auto marginSizePt = UnitsOfMeasure::convertToPt(reactionComponentMarginSize, reactionComponentMarginSizeUnit, ppi);
            auto bondLengthPt = UnitsOfMeasure::convertToPt(bondLength, bondLengthUnit, ppi);

            return marginSizePt / bondLengthPt;
        }
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif //__metalayout_h__
