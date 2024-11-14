#include "ring/Submission.hpp"

coContext::Submission::Submission(const int fileDescriptor, const unsigned int flags, const unsigned short ioPriority,
                                  parameterType &&parameter) noexcept :
    fileDescriptor{fileDescriptor}, flags{flags}, ioPriority{ioPriority}, parameter{std::move(parameter)} {}

auto coContext::Submission::setUserData(unsigned long userData) noexcept -> void { this->userData = userData; }

auto coContext::Submission::setSqe(io_uring_sqe *const sqe) const noexcept -> void {
    switch (static_cast<Type>(this->parameter.index())) {
        case Type::close:
            io_uring_prep_close(sqe, this->fileDescriptor);

            break;
    }

    io_uring_sqe_set_flags(sqe, this->flags);
    sqe->ioprio |= this->ioPriority;
    io_uring_sqe_set_data64(sqe, this->userData);
}
