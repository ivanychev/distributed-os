
struct context_common_s {
};

struct context_common_s context_common;

// ATTN context
struct context_attn_s {
  int  ready;
  context_attn_s() {
     ready = 0;
  }
};

struct context_attn_s context_attn;

enum class State {
    kRegular,
    kInElection,
    kCoordinator
};

struct context_bully_s {
    State state{State::kRegular};
    int coordinator{-1};
    int election_start{0};
    int time{0};

    bool isCoordinator() const {
        return state == State::kCoordinator;
    };
};

struct context_bully_s context_bully;
