#pragma once

#include <NativeJSExposer.h>

class NativeBindingInterface_{{name}}
{
public:
    {% if ctor %}
    static NativeBindingInterface_{{name}} *Constructor();
    {% endif %}
    
private:
};
