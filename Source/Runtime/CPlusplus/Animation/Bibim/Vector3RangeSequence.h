#pragma once
#ifndef __BIBIM_VECTOR3RANGESEQUENCE_H__
#define __BIBIM_VECTOR3RANGESEQUENCE_H__

#   include <Bibim/FWD.h>
#   include <Bibim/Asset.h>
#   include <Bibim/Range.h>
#   include <Bibim/Vector3.h>

    namespace Bibim
    {
        struct Vector3RangeKeyframe
        {
            typedef Range<Vector3> ValueType;

            ValueType Value;
            float Duration;
            float StartTime;

            Vector3RangeKeyframe();
            Vector3RangeKeyframe(ValueType value, float duration);
        };

        class Vector3RangeSequence : public Asset
        {
            public:
                typedef Vector3RangeKeyframe KeyframeType;

            public:
                Vector3RangeSequence();
                virtual ~Vector3RangeSequence();

                void AddKeyframe(const KeyframeType& item);
                void InsertKeyframe(int index, const KeyframeType& item);
                void InsertKeyframe(float time, const KeyframeType& item);
                void RemoveKeyframe(int index);
                void RemoveKeyframe(float time);
                void RemoveAllKeyframes();

                int GetNumberOfKeyframes();
                const KeyframeType& GetKeyframe(int index);
                const KeyframeType& GetKeyframe(float time);
                int GetKeyframeIndex(float time);
                int GetKeyframeIndex(float time, int startIndex);
                void SetKeyframe(int index, const KeyframeType& item);

                float GetDuration();
           
            private:
                SequenceTemplate<KeyframeType>* s;
        };
    }

#endif