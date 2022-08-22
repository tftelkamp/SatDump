/**********************************************************************
 * This file is used for testing random stuff without running the
 * whole of SatDump, which comes in handy for debugging individual
 * elements before putting them all together in modules...
 *
 * If you are an user, ignore this file which will not be built by
 * default, and if you're a developper in need of doing stuff here...
 * Go ahead!
 *
 * Don't judge the code you might see in there! :)
 **********************************************************************/

#include "logger.h"
#include <fstream>

#include "common/dsp/complex.h"

int main(int argc, char *argv[])
{
    initLogger();

#if 1
    std::ifstream input_frm(argv[1]);
    std::ofstream output_frm("test_mdl.frm");

    uint8_t cadu[4640];
    uint32_t mdl_packets[128];

    std::vector<uint8_t> test_v;

    while (!input_frm.eof())
    {
        input_frm.read((char *)cadu, 464);

        memset(mdl_packets, 0, sizeof(uint32_t) * 128);

        // Repack into 29 (WTF NOAA!?) words
        {
            int bits_in_word = 0;
            int curr_word = 0;
            for (int iby = 0; iby < 464; iby++)
            {
                for (int ibi = 0; ibi < 8; ibi++)
                {
                    uint8_t bit = (cadu[iby] >> (7 - ibi)) & 1;

                    mdl_packets[curr_word] = mdl_packets[curr_word] << 1 | bit;
                    bits_in_word++;

                    if (bits_in_word == 29)
                    {
                        curr_word++;
                        bits_in_word = 0;
                    }
                }
            }
        }

        for (int i = 0; i < 128; i++)
        {
            uint8_t marker = (mdl_packets[i] >> 24) & 0b11111;

            printf("VCID %d\n", marker);
            // logger->critical(marker);

            // 4 & 7 = 0xb48b6f

            if (marker == 16)
            {
                if ((mdl_packets[i] >> 24) & 0b1 == 1)
                {
                    printf("SIZE %d\n", (int)test_v.size());
                    test_v.clear();
                }

                uint8_t buf[4];
                buf[0] = (mdl_packets[i] >> 24) & 0xFF;
                buf[0] <<= 3;
                buf[1] = (mdl_packets[i] >> 16) & 0xFF;
                buf[2] = (mdl_packets[i] >> 8) & 0xFF;
                buf[3] = (mdl_packets[i] >> 0) & 0xFF;
                test_v.push_back(buf[1]);
                test_v.push_back(buf[2]);
                test_v.push_back(buf[3]);
                output_frm.write((char *)&buf[1], 3);
            }
        }

        // logger->info(cadu[marker * 30]);
        // if (cadu[marker * 30] == 168)
        //  output_frm.write((char *)cadu, 464);
    }
#endif

#if 0
    std::ifstream input_frm(argv[1]);
    std::ofstream output_frm(argv[2]);

    complex_t iq_buffer[8192];
    int8_t buffer_mag_phase[8192 * 2];

    while (!input_frm.eof())
    {
        input_frm.read((char *)iq_buffer, 8192 * sizeof(complex_t));

        for (int i = 0; i < 8192; i++)
        {
            complex_t &v = iq_buffer[i];

            float mag = v.norm();
            float phase = atan2f(v.imag, v.real);

            mag = (mag * 127) / 2.0;
            phase = (phase * 127) / M_PI;

            if (mag > 127)
                mag = 1227;
            if (mag < -127)
                mag = -127;

            buffer_mag_phase[i * 2 + 0] = mag;
            buffer_mag_phase[i * 2 + 1] = phase;
        }

        output_frm.write((char *)buffer_mag_phase, 8192 * 2);
    }
#endif

#if 0
    std::ifstream input_frm(argv[1]);
    std::ofstream output_frm(argv[2]);

    complex_t iq_buffer[8192];
    int8_t buffer_mag_phase[8192 * 2];

    while (!input_frm.eof())
    {
        input_frm.read((char *)buffer_mag_phase, 8192 * 2);

        for (int i = 0; i < 8192; i++)
        {
            float mag = buffer_mag_phase[i * 2 + 0];
            float phase = buffer_mag_phase[i * 2 + 1];

            mag = (mag / 127) * 2.0;
            phase = (phase / 127) * M_PI;

            iq_buffer[i].real = sinf(phase) * mag;
            iq_buffer[i].real = cosf(phase) * mag;
        }

        output_frm.write((char *)iq_buffer, 8192 * sizeof(complex_t));
    }
#endif
}
