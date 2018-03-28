/*
 *    Copyright (C) 2018
 *    Matthias P. Braendli (matthias.braendli@mpb.li)
 *
 *    This file is part of the welle.io.
 *
 *    welle.io is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    welle.io is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with welle.io; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include "dab-constants.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <complex>
#include <cstddef>
#include "fft.h"

using complexf = std::complex<float>;

struct CombPattern {
    CombPattern() = default;
    CombPattern(int c, int p) :
        comb(c), pattern(p) {}
    int comb = 0; // From 0 to 24
    int pattern = 0; // From 0 to 70
};

// Make CombPattern satisfy Hash and Compare
bool operator==(const CombPattern& lhs, const CombPattern& rhs);

namespace std {
    template<> struct hash<CombPattern> {
        typedef CombPattern argument_type;
        typedef std::size_t result_type;
        result_type operator()(argument_type const& cp) const noexcept
        {
            return cp.comb * 100 + cp.pattern;
        }
    };
}

using carrier_t = int;

class TIIDecoder {
    public:
        TIIDecoder(const DABParams& params);
        ~TIIDecoder();
        TIIDecoder(const TIIDecoder& other) = delete;
        TIIDecoder& operator=(const TIIDecoder& other) = delete;

        void push_nullsymbol(const std::vector<complexf>& null);
        void push_prs(const std::vector<complexf>& prs);

    private:
        void run(void);

        const DABParams& m_params;

        std::vector<complexf> m_null;
        std::vector<complexf> m_prs;

        std::unordered_map<carrier_t, std::unordered_set<CombPattern> >
            m_cp_per_carrier;

        enum class State { Idle, NullReady, PrsReady, BothReady, Abort };

        std::thread m_thread;
        std::mutex m_state_mutex;
        std::condition_variable m_state_changed;
        State m_state = State::Idle;

        common_fft m_fft_null;
        common_fft m_fft_prs;

};

