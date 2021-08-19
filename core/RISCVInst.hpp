// <RISCVInst.h> -*- C++ -*-


#pragma once

#include "sparta/decode/DecoderBase.hpp"
#include "sparta/memory/AddressTypes.hpp"
#include "sparta/resources/SharedData.hpp"
#include "sparta/pairs/SpartaKeyPairs.hpp"
#include "sparta/simulation/State.hpp"
#include "sparta/utils/SpartaSharedPointer.hpp"

#include <cstdlib>
#include <ostream>
#include <map>

namespace olympia_core
{
    /*!
    * \class RISCVInst
    * \brief Example instruction that flows through the example/CoreModel
    */

    // Forward declaration of the Pair Definition class is must as we are friending it.
    class RISCVInstPairDef;

    class RISCVInst {
    public:

        // The modeler needs to alias a type called "SpartaPairDefinitionType" to the Pair Definition class  of itself
        using SpartaPairDefinitionType = RISCVInstPairDef;

        enum class Status : std::uint16_t{
            FETCHED = 0,
            __FIRST = FETCHED,
            DECODED,
            RENAMED,
            SCHEDULED,
            COMPLETED,
            RETIRED,
            __LAST
        };

        enum class TargetUnit : std::uint16_t{
            ALU0,
            ALU1,
            FPU,
            BR,
            LSU,
            ROB, // Instructions that go right to retire
            N_TARGET_UNITS
        };

        struct StaticInfo {
            sparta::decode::DecoderBase decode_base;
            TargetUnit unit;
            uint32_t execute_time;
            bool is_store_inst;
        };

        using InstStatus = sparta::SharedData<Status>;

        RISCVInst(const sparta::decode::DecoderBase & static_inst,
                    TargetUnit unit,
                    uint32_t execute_time,
                    bool isStore,
                    const sparta::Clock * clk,
                    Status state) :
            static_inst_(static_inst),
            unit_(unit),
            execute_time_(execute_time),
            isStoreInst_(isStore),
            status_("inst_status", clk, state),
            status_state_(state) {}

        RISCVInst(const StaticInfo & info,
                    const sparta::Clock * clk,
                    Status state = Status::FETCHED) :
            RISCVInst(info.decode_base,
                        info.unit,
                        info.execute_time,
                        info.is_store_inst,
                        clk,
                        state)
        {}

        const sparta::decode::DecoderBase & getStaticInst() const {
            return static_inst_;
        }

        const Status & getStatus() const {
            return status_state_.getEnumValue();
        }

        bool getCompletedStatus() const {
            return getStatus() == olympia_core::RISCVInst::Status::COMPLETED;
        }

        void setStatus(Status status) {
            status_state_.setValue(status);
            status_.write(status);
            if(getStatus() == olympia_core::RISCVInst::Status::COMPLETED) {
                if(ev_retire_ != 0) {
                    ev_retire_->schedule();
                }
            }
        }

        const TargetUnit& getUnit() const {
            return unit_;
        }

        void setLast(bool last, sparta::Scheduleable * rob_retire_event) {
            ev_retire_ = rob_retire_event;
            is_last_ = last;

            if(status_.isValidNS() && status_.readNS() == olympia_core::RISCVInst::Status::COMPLETED) {
                ev_retire_->schedule();
            }
        }

        void setVAdr(uint64_t vaddr) {
            vaddr_ = vaddr;
        }

        void setUniqueID(uint64_t uid) {
            unique_id_ = uid;
        }

        // This is a function which will be added in the SPARTA_ADDPAIRs API.
        uint64_t getUniqueID() const {
            return unique_id_;
        }

        void setSpeculative(bool spec) {
            is_speculative_ = spec;
        }

        const char* getMnemonic() const { return static_inst_.mnemonic; }
        uint32_t getOpCode() const { return static_inst_.encoding; }
        uint64_t getVAdr() const { return vaddr_; }
        uint64_t getRAdr() const { return vaddr_ | 0x3000; } // faked
        uint64_t getParentId() const { return 0; }
        uint32_t getExecuteTime() const { return execute_time_; }
        bool isSpeculative() const { return is_speculative_; }
        bool isStoreInst() const { return isStoreInst_; }

    private:

        const sparta::decode::DecoderBase static_inst_;
        TargetUnit unit_;
        const uint32_t execute_time_ = 0;
        bool isStoreInst_ = false;
        sparta::memory::addr_t vaddr_ = 0;
        bool is_last_ = false;
        uint64_t unique_id_ = 0; // Supplied by Fetch
        bool is_speculative_ = false; // Is this instruction soon to be flushed?
        sparta::Scheduleable * ev_retire_ = nullptr;
        InstStatus status_;
        sparta::State<Status> status_state_;
    };

    extern sparta::SpartaSharedPointer<RISCVInst>::SpartaSharedPointerAllocator example_inst_allocator;

    inline std::ostream & operator<<(std::ostream & os, const RISCVInst & inst) {
        os << inst.getMnemonic();
        return os;
    }

    typedef sparta::SpartaSharedPointer<RISCVInst> RISCVInstPtr;
    inline std::ostream & operator<<(std::ostream & os, const RISCVInstPtr & inst) {
        os << *inst;
        return os;
    }

    inline std::ostream & operator<<(std::ostream & os, const RISCVInst::TargetUnit & unit) {
        switch(unit) {
            case RISCVInst::TargetUnit::ALU0:
                os << "ALU0";
                break;
            case RISCVInst::TargetUnit::ALU1:
                os << "ALU1";
                break;
            case RISCVInst::TargetUnit::FPU:
                os << "FPU";
                break;
            case RISCVInst::TargetUnit::BR:
                os << "BR";
                break;
            case RISCVInst::TargetUnit::LSU:
                os << "LSU";
                break;
            case RISCVInst::TargetUnit::ROB:
                os << "ROB";
                break;
            case RISCVInst::TargetUnit::N_TARGET_UNITS:
                throw sparta::SpartaException("N_TARGET_UNITS cannot be a valid enum state.");
        }
        return os;
    }

    inline std::ostream & operator<<(std::ostream & os, const RISCVInst::Status & status) {
        switch(status) {
            case RISCVInst::Status::FETCHED:
                os << "FETCHED";
                break;
            case RISCVInst::Status::DECODED:
                os << "DECODED";
                break;
            case RISCVInst::Status::RENAMED:
                os << "RENAMED";
                break;
            case RISCVInst::Status::SCHEDULED:
                os << "SCHEDULED";
                break;
            case RISCVInst::Status::COMPLETED:
                os << "COMPLETED";
                break;
            case RISCVInst::Status::RETIRED:
                os << "RETIRED";
                break;
            case RISCVInst::Status::__LAST:
                throw sparta::SpartaException("__LAST cannot be a valid enum state.");
        }
        return os;
    }

    /*!
    * \class RISCVInstPairDef
    * \brief Pair Definition class of the Example instruction that flows through the example/CoreModel
    */
    // This is the definition of the PairDefinition class of RISCVInst.
    // This PairDefinition class could be named anything but it needs to
    // inherit publicly from sparta::PairDefinition templatized on the actual class RISCVInst.
    class RISCVInstPairDef : public sparta::PairDefinition<RISCVInst>{
    public:

        // The SPARTA_ADDPAIRs APIs must be called during the construction of the PairDefinition class
        RISCVInstPairDef() : PairDefinition<RISCVInst>(){
            SPARTA_INVOKE_PAIRS(RISCVInst);
        }
        SPARTA_REGISTER_PAIRS(SPARTA_ADDPAIR("DID",      &RISCVInst::getUniqueID),
                              SPARTA_ADDPAIR("uid",      &RISCVInst::getUniqueID),
                              SPARTA_ADDPAIR("mnemonic", &RISCVInst::getMnemonic),
                              SPARTA_ADDPAIR("complete", &RISCVInst::getCompletedStatus),
                              SPARTA_ADDPAIR("unit",     &RISCVInst::getUnit),
                              SPARTA_ADDPAIR("latency",  &RISCVInst::getExecuteTime),
                              SPARTA_ADDPAIR("raddr",    &RISCVInst::getRAdr, std::ios::hex),
                              SPARTA_ADDPAIR("vaddr",    &RISCVInst::getVAdr, std::ios::hex));
    };
}
