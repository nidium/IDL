#pragma once

#include <NativeJSExposer.h>

class NativeJS{{classname}} : public NativeJSExposer<NativeJS{{classname}}>
{{
    public:
        NativeJS{{classname}}(JS::HandleObject obj, JSContext *cx);
        virtual ~NativeJS{{classname}}();
    private:
}};
