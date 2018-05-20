#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>

#include "contextes.h"
#include "DSSimul.h"

namespace bully {

const int kTimeThreshold = 5;

using HandlerType = std::function<void(Process* process, const Message& message)>;
using HandlersMap = const std::unordered_map<std::string, HandlerType>;

bool HaveTopId(Process* process) {
    const auto& neighbours = process->neibs();
    return std::all_of(neighbours.begin(), neighbours.end(),
                       [process](int neighbour) {
                           return neighbour < process->node;
                       });
}

void Log(Process* process, const Message& message, const std::string& text) {
    std::stringstream ss;
    ss << process->node
       << " <- "
       << message.from
       << " "
       << process->context_bully.time
       << " " <<  text
       << "\n";
    std::cout << ss.str();
}


void StartElection(Process* process) {
    auto& context = process->context_bully;
    const auto& neighbours = process->neibs();
    auto* network = process->networkLayer;

    context.SetInElection();
    for (auto neighbour_id : neighbours) {
        if (neighbour_id > process-> node) {
            network->send(process->node, neighbour_id, Message("BULLY_ELECTION"));
        }
    }
}

void BroadcastCoordinator(Process* process) {
    auto network = process->networkLayer;
    process->context_bully.state = Process::State::kCoordinator;

    process->context_bully.coordinator = process->node;
    for (auto neighbour_id : process->neibs()) {
        network->send(process->node, neighbour_id, Message("BULLY_COORDINATOR"));
    }
}

void Alive(Process* process, const Message& message) {
    if (process->context_bully.isInElection()) {
        process->context_bully.state = Process::State::kWaitForWinner;
        process->context_bully.wait_start = process->context_bully.time;
    } else {
        process->context_bully.state = Process::State::kRegular;
    }
}

void Init(Process* process, const Message& message) {
    bool have_top_id = HaveTopId(process);
    auto network = process->networkLayer;
    auto neighbours = process->neibs();

    if (have_top_id) {
        for (auto neighbour_id : neighbours) {
            network->send(process->node, neighbour_id, Message("BULLY_COORDINATOR"));
        }
    } else {
        StartElection(process);
    }
}

void Election(Process* process, const Message& message) {
    auto* network = process->networkLayer;
    const auto& neighbours = process->neibs();
    auto& context = process->context_bully;

    if (message.from < process->node) {
        network->send(process->node, message.from, Message("BULLY_ALIVE"));
        StartElection(process);
    }
}

void Coordinator(Process* process, const Message& message) {
    if (process->node > message.from) {
        StartElection(process);
    } else {
        process->context_bully.coordinator = message.from;
        process->context_bully.state = Process::State::kRegular;
    }
}

void Time(Process* process, const Message& message) {
    process->context_bully.time = message.getInt();

    if (process->context_bully.isInElection()) {
        if (process->context_bully.ElectionTimeRanOut(kTimeThreshold)) {
            BroadcastCoordinator(process);
        }
    } else if (process->context_bully.isWaitingForWinner()) {
        if (process->context_bully.WaitTimeRanOut(kTimeThreshold)) {
            StartElection(process);
        }
    }

}

HandlersMap kBullyHandlers{
    {"BULLY_INIT", Init},
    {"BULLY_ALIVE", Alive},
    {"BULLY_ELECTION", Election},
    {"BULLY_COORDINATOR", Coordinator},
    {"*TIME", Time},
};

}  // namespace bully


int workFunction_BULLY(Process* process, Message message) {
    std::string text = message.getString();
    if (!Process::isMyMessage("BULLY", text)) {
        return false;
    }
    auto* network_lawyer = process->networkLayer;
    auto handler_it = bully::kBullyHandlers.find(text);
    bully::Log(process, message, text);

    if (handler_it == bully::kBullyHandlers.end()) {
        std::cerr << "Invalid message";
        return 0;
    }
    auto handler = handler_it->second;
    handler(process, message);
    return 1;
}

int main(int argc, char** argv) {
    std::string config = argc > 1 ? argv[1] : "config.data";
    World w;

    w.registerWorkFunction("BULLY", workFunction_BULLY);

    if (w.parseConfig(config)) {
        this_thread::sleep_for(chrono::milliseconds(3000000));
    } else {
        std::cout << "can't open file " << config << "\n";
    }

    return 0;
}