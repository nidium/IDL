{% macro arglst(args) %}
{% for arg in args %}{{arg.idlType.idlType|ctype}} {{arg.name}}{{ ', ' if not loop.last }}{% endfor %}
{% endmacro %}

{% macro jsval2c(jval, need, dest) %}
{% if need == 'cstring' %}
JS::RootedString __curstr(cx, JS::ToString(cx, {{jval}}));
if (!__curstr) {
    JS_ReportError(cx, "TypeError");
    return false;
}
JSAutoByteString __curstr_c;
__curstr_c.encodeUtf8(cx, __curstr);

char *{{dest}} = __curstr_c.ptr();
{% endif %}
{% endmacro %}

#pragma once

#include <NativeJSExposer.h>

class NativeBindingInterface_{{name}}
{
public:
    {% if ctor %}
    static NativeBindingInterface_{{name}} *Constructor();
    {% endif %}
    
    {% for attr in members %}
    {% if attr.type == 'operation'%}
    virtual {{ attr.idlType.idlType|ctype }} {{attr.name}}({{arglst(attr.arguments)}})=0;
    {%endif%}
    {% endfor %}

    /* JS Natives */

    {% for attrName, attrData in operations %}
    static bool js_{{attrName}}(JSContext *cx, unsigned argc, JS::Value *vp)
    {
        JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
        JS::RootedObject caller(cx, JS_THIS_OBJECT(cx, vp));

        unsigned argcMin = (({{attrData.maxArgs}} > argc) ? (argc) : ({{attrData.maxArgs}}))

        switch (argcMin) {
            {% for op in attrData.lst %}
            case {{op.arguments.length}}:
            {
                {{jsval2c('args[0]', 'cstring', foo)}} 
                break;
            }
            {% endfor %}
        }

        return true;
    }
    {% endfor %}

    static void JSFinalize(JSFreeOp *fop, JSObject *obj)
    {

    }
private:
};

static JSClass Socket_class = {
    "{{ className }}", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NativeBindingInterface_{{name}}::JSFinalize,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

static JSFunctionSpec HTTPResponse_funcs[] = {
    {% for attr in members %}
    {% if attr.type == 'operation'%}
    JS_FN("{{attr.name}}", NativeBindingInterface_{{name}}::js_{{attr.name}}, 0, 0),
    {%endif%}
    {% endfor %}
    JS_FS_END
};
