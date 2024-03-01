#pragma once

// Includes
//------------------------------------------------------------------------------
// System
#include <exception>

class NotImplementedException : public std::exception 
{
public:    
    const char* what() const noexcept override 
    {
        return "This functionality has not been implemented yet";
    }
};