#pragma once

namespace scale::lcd {
class BaseWidget {
   public:
    virtual void render() = 0;
};
}  // namespace scale::lcd