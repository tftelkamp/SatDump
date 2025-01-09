#pragma once
#include <set>
#include <array>
#include <map>
#include "common/ccsds/ccsds.h"
#include "common/image/image.h"
#include "lrpt/segment.h"

namespace meteor
{
    namespace msumr
    {
        namespace lrpt
        {
            class MSUMRReader
            {
            private:
                std::map<uint32_t, Segment> segments[6];
                uint32_t firstSeg[6], rollover[6], lastSeq[6], offset[6], lastSeg[6];

                time_t dayValue;

                const bool meteorm2x_mode;

            public:
                MSUMRReader(bool meteorm2x_mode);
                ~MSUMRReader();
                void work(ccsds::CCSDSPacket &packet);
                image::Image getChannel(int channel, size_t max_correct = 0);

                int lines[6];
                std::vector<double> timestamps;
            };
        } // namespace lrpt
    }     // namespace msumr
} // namespace meteor