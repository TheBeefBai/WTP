#ifndef _EXP_VOTE_PAGE_HPP
#define _EXP_VOTE_PAGE_HPP

#include <kamek.hpp>
#include <MarioKartWii/UI/Page/Other/Votes.hpp>

namespace Pulsar {
namespace UI {

class ExpVotePage : public Pages::Vote {
    void BeforeControlUpdate() override;
};

} // namespace UI
} // namespace Pulsar

#endif // _EXP_VOTE_PAGE_HPP