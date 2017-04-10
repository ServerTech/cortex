/*
    Cortex - Self-learning Chess Engine
    @filename sliding_lookupgen.cc
    @author Shreyas Vinod
    @version 1.0.0

    @brief Generates well-formatted files with lookup tables for sliding
           pieces.

    Generates and dumps sliding piece (queen, bishop and rook) lookup tables
    into well-formatted text files. These values are later used in the
    move generator. Based on Little-Endian Rank-File mapping (LERF).

    ******************** VERSION CONTROL ********************
    * 11/07/2015 File created.
    * 11/07/2015 0.1.2 Initial version.
    * 08/11/2015 0.1.1 Replaced 'FILE' with 'B_FILE' and 'RANK' with 'B_RANK'
                       to prevent collisions with the C's 'FILE'.
    * 10/04/2017 1.0.0 Release 'Primeval'
*/

/**
    @file
    @filename sliding_lookupgen.cc
    @author Shreyas Vinod

    @brief Generates well-formatted files with lookup tables for sliding
           pieces.

    Generates and dumps sliding piece (queen, bishop and rook) lookup tables
    into well-formatted text files. These values are later used in the
    move generator. Based on Little-Endian Rank-File mapping (LERF).
*/

#include <fstream>
#include <iomanip>

typedef unsigned long long uint64;

int main()
{
    uint64 B_FILE[8] = {
        0x8080808080808080ULL, 0x4040404040404040ULL, 0x2020202020202020ULL,
        0x1010101010101010ULL, 0x0808080808080808ULL, 0x0404040404040404ULL,
        0x0202020202020202ULL, 0x0101010101010101ULL
    }; // Filled with ones on the corresponding file, starting with A.

    uint64 B_RANK[8] = {
        0x00000000000000ffULL, 0x000000000000ff00ULL, 0x0000000000ff0000ULL,
        0x00000000ff000000ULL, 0x000000ff00000000ULL, 0x0000ff0000000000ULL,
        0x00ff000000000000ULL, 0xff00000000000000ULL
    }; // Filled with ones on the corresponding rank, starting with 1.

    uint64 queen_sq, bishop_sq, rook_sq;
    uint64 ne_bishop, nw_bishop, se_bishop, sw_bishop;
    uint64 n_rook, s_rook, e_rook, w_rook;
    uint64 keep_files = 0ULL;

    std::ofstream queen, rook, bishop;
    std::ofstream bishop_ne, bishop_nw, bishop_se, bishop_sw;
    std::ofstream rook_north, rook_south, rook_east, rook_west;
    queen.open("queen_lookup_table.txt");
    rook.open("rook_lookup_table.txt");
    bishop.open("bishop_lookup_table.txt");
    bishop_ne.open("bishop_ne_lookup_table.txt");
    bishop_nw.open("bishop_nw_lookup_table.txt");
    bishop_se.open("bishop_se_lookup_table.txt");
    bishop_sw.open("bishop_sw_lookup_table.txt");
    rook_north.open("rook_north_lookup_table.txt");
    rook_south.open("rook_south_lookup_table.txt");
    rook_east.open("rook_east_lookup_table.txt");
    rook_west.open("rook_west_lookup_table.txt");

    for(int i = 0; i < 64; i++) // Iterate over every square
    {
        // Bishop (northeast)

        ne_bishop = 1ULL << i;
        ne_bishop = (ne_bishop << 9) | (ne_bishop << 18) | (ne_bishop << 27) |
               (ne_bishop << 36) | (ne_bishop << 45) | (ne_bishop << 54) |
               (ne_bishop << 63);

        for(int j = (7 - (i % 8)); j >= 0; j--)
        {
            keep_files |= B_FILE[j];
        }

        ne_bishop &= keep_files;
        keep_files = 0ULL;

        bishop_ne << "0x" << std::setfill('0') <<
                     std::setw(16) << std::hex << ne_bishop << "ULL";
        if(i != 63) bishop_ne << ",";
        if((i != 0) && (((i + 1) % 3) == 0)) bishop_ne << std::endl;
        else if(i != 63) bishop_ne << " ";

        // Bishop (northwest)

        nw_bishop = 1ULL << i;
        nw_bishop = (nw_bishop << 7) | (nw_bishop << 14) | (nw_bishop << 21) |
               (nw_bishop << 28) | (nw_bishop << 35) | (nw_bishop << 42) |
               (nw_bishop << 49);

        for(int j = (7 - (i % 8)); j <= 7; j++)
        {
            keep_files |= B_FILE[j];
        }

        nw_bishop &= keep_files;
        keep_files = 0ULL;

        bishop_nw << "0x" << std::setfill('0') <<
                     std::setw(16) << std::hex << nw_bishop << "ULL";
        if(i != 63) bishop_nw << ",";
        if((i != 0) && (((i + 1) % 3) == 0)) bishop_nw << std::endl;
        else if(i != 63) bishop_nw << " ";

        // Bishop (southeast)

        se_bishop = 1ULL << i;
        se_bishop = (se_bishop >> 7) | (se_bishop >> 14) | (se_bishop >> 21) |
               (se_bishop >> 28) | (se_bishop >> 35) | (se_bishop >> 42) |
               (se_bishop >> 49);

        for(int j = (7 - (i % 8)); j >= 0; j--)
        {
            keep_files |= B_FILE[j];
        }

        se_bishop &= keep_files;
        keep_files = 0ULL;

        bishop_se << "0x" << std::setfill('0') <<
                     std::setw(16) << std::hex << se_bishop << "ULL";
        if(i != 63) bishop_se << ",";
        if((i != 0) && (((i + 1) % 3) == 0)) bishop_se << std::endl;
        else if(i != 63) bishop_se << " ";

        // Bishop (southwest)

        sw_bishop = 1ULL << i;
        sw_bishop = (sw_bishop >> 9) | (sw_bishop >> 18) | (sw_bishop >> 27) |
               (sw_bishop >> 36) | (sw_bishop >> 45) | (sw_bishop >> 54) |
               (sw_bishop >> 63);

        for(int j = (7 - (i % 8)); j <= 7; j++)
        {
            keep_files |= B_FILE[j];
        }

        sw_bishop &= keep_files;
        keep_files = 0ULL;

        bishop_sw << "0x" << std::setfill('0') <<
                     std::setw(16) << std::hex << sw_bishop << "ULL";
        if(i != 63) bishop_sw << ",";
        if((i != 0) && (((i + 1) % 3) == 0)) bishop_sw << std::endl;
        else if(i != 63) bishop_sw << " ";

        // Rook (north)

        n_rook = 1ULL << i;
        n_rook = (n_rook << 8) | (n_rook << 16) | (n_rook << 24) |
               (n_rook << 32) | (n_rook << 40) | (n_rook << 48) |
               (n_rook << 56);
        n_rook &= B_FILE[7 - (i % 8)];

        rook_north << "0x" << std::setfill('0') <<
                      std::setw(16) << std::hex << n_rook << "ULL";
        if(i != 63) rook_north << ",";
        if((i != 0) && (((i + 1) % 3) == 0)) rook_north << std::endl;
        else if(i != 63) rook_north << " ";

        // Rook (south)

        s_rook = 1ULL << i;
        s_rook = (s_rook >> 8) | (s_rook >> 16) | (s_rook >> 24) |
               (s_rook >> 32) | (s_rook >> 40) | (s_rook >> 48) |
               (s_rook >> 56);
        s_rook &= B_FILE[7 - (i % 8)];

        rook_south << "0x" << std::setfill('0') <<
                      std::setw(16) << std::hex << s_rook << "ULL";
        if(i != 63) rook_south << ",";
        if((i != 0) && (((i + 1) % 3) == 0)) rook_south << std::endl;
        else if(i != 63) rook_south << " ";

        // Rook (east)

        e_rook = 1ULL << i;
        e_rook = (e_rook << 1) | (e_rook << 2) | (e_rook << 3) |
               (e_rook << 4) | (e_rook << 5) | (e_rook << 5) |
               (e_rook << 6) | (e_rook << 7);
        e_rook &= B_RANK[i / 8];

        rook_east << "0x" << std::setfill('0') <<
                     std::setw(16) << std::hex << e_rook << "ULL";
        if(i != 63) rook_east << ",";
        if((i != 0) && (((i + 1) % 3) == 0)) rook_east << std::endl;
        else if(i != 63) rook_east << " ";

        // Rook (west)

        w_rook = 1ULL << i;
        w_rook = (w_rook >> 1) | (w_rook >> 2) | (w_rook >> 3) |
               (w_rook >> 4) | (w_rook >> 5) | (w_rook >> 5) |
               (w_rook >> 6) | (w_rook >> 7);
        w_rook &= B_RANK[i / 8];

        rook_west << "0x" << std::setfill('0') <<
                     std::setw(16) << std::hex << w_rook << "ULL";
        if(i != 63) rook_west << ",";
        if((i != 0) && (((i + 1) % 3) == 0)) rook_west << std::endl;
        else if(i != 63) rook_west << " ";

        // Bishop

        bishop_sq = ne_bishop | nw_bishop | se_bishop | sw_bishop;

        bishop << "0x" << std::setfill('0') <<
                     std::setw(16) << std::hex << bishop_sq << "ULL";
        if(i != 63) bishop << ",";
        if((i != 0) && (((i + 1) % 3) == 0)) bishop << std::endl;
        else if(i != 63) bishop << " ";

        // Rook

        rook_sq = n_rook | s_rook | e_rook | w_rook;

        rook << "0x" << std::setfill('0') <<
                     std::setw(16) << std::hex << rook_sq << "ULL";
        if(i != 63) rook << ",";
        if((i != 0) && (((i + 1) % 3) == 0)) rook << std::endl;
        else if(i != 63) rook << " ";

        // Queen

        queen_sq = bishop_sq | rook_sq;

        queen << "0x" << std::setfill('0') <<
                     std::setw(16) << std::hex << queen_sq << "ULL";
        if(i != 63) queen << ",";
        if((i != 0) && (((i + 1) % 3) == 0)) queen << std::endl;
        else if(i != 63) queen << " ";
    }

    queen.close();
    rook.close();
    bishop.close();
    bishop_ne.close();
    bishop_nw.close();
    bishop_se.close();
    bishop_sw.close();
    rook_north.close();
    rook_south.close();
    rook_east.close();
    rook_west.close();
}