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
{% else %}
{{need|ctype}} {{dest}};
                if (!JS::{{ need|convert }}(cx, {{jval}}, &{{dest}})) {
                    JS_ReportError(cx, "TypeError");
                    return false;
                }
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
                /* Start arguments convertion */

                {% for arg in op.arguments %}
                /* Handle argument #{{ loop.index0 }} of type "{{ arg.idlType.idlType }}" */
                {{jsval2c('args['~ loop.index0 ~']', arg.idlType.idlType, 'inArg_' ~ loop.index0)}} 
                {% endfor %}
                /* End of arguments convertion */

                obj->{{attrName}}({% for i in range(0, op.arguments.length) %}inArg_{{i}}{{ ', ' if not loop.last }}{%endfor%});
                break;
            }
            {% endfor %}
            default:
                JS_ReportError(cx, "TypeError: wrong number of arguments";)
                return false;
                break;
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
    {% for attrName, attrData in operations %}
    JS_FN("{{attrName}}", NativeBindingInterface_{{name}}::js_{{attrName}}, {{attrData.maxArgs}}, 0),
    {% endfor %}
    JS_FS_END
};
