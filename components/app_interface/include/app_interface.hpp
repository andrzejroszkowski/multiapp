#pragma once
#include <string>

class AppInterface {
public:
    virtual ~AppInterface() = default;

    // Hooki cyklu zycia sub-aplikacji
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual std::string get_name() const = 0;
};
