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
#include <jsapi.h>

class NativeBindingInterface_{{name}}
{
public:

    template <typename T>
    static bool registerObject(JSContext *cx, JS::HandleObject exports = JS::NullPtr());

    {% if ctor %}
        /* These static(s) must be implemented */
        {% for constructor in constructors.lst %}
        //static NativeBindingInterface_{{name}} *Constructor({{arglst(constructor.arguments)}});
        {% endfor %}

        template <typename T>
        static bool js_{{name}}_Constructor(JSContext *cx, unsigned argc, JS::Value *vp);
    {% endif %}
    
    {% for attr in members %}
        {% if attr.type == 'operation'%}
            virtual {{ attr.idlType.idlType|ctype }} {{attr.name}}({{arglst(attr.arguments)}})=0;
        {%endif%}
    {% endfor %}

    /* JS Natives */

    {% for attrName, attrData in operations %}
        static bool js_{{attrName}}(JSContext *cx, unsigned argc, JS::Value *vp);
    {% endfor %}

    static void JSFinalize(JSFreeOp *fop, JSObject *obj)
    {

    }
private:
};

static JSClass {{ className }}_class = {
    "{{ className }}", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NativeBindingInterface_{{name}}::JSFinalize,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

static JSFunctionSpec {{ className }}_funcs[] = {
    {% for attrName, attrData in operations %}
        JS_FN("{{attrName}}", NativeBindingInterface_{{name}}::js_{{attrName}}, {{attrData.maxArgs}}, 0),
    {% endfor %}
    JS_FS_END
};

{% if ctor %}
template <typename T>
bool NativeBindingInterface_{{name}}::js_{{name}}_Constructor(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    if (!args.isConstructing()) {
        JS_ReportError(cx, "Bad constructor");
        return false;
    }

    unsigned argcMin = (({{constructors.maxArgs}} > argc) ? (argc) : ({{constructors.maxArgs}}));

    switch (argcMin) {
        {% for op in constructors.lst %}
            case {{op.arguments.length}}:
            {
                /* Start arguments convertion */

                {% for arg in op.arguments %}
                    /* Handle argument #{{ loop.index0 }} of type "{{ arg.idlType.idlType }}" */
                    {% if not arg.idlType.nullable %}
                        if (args[{{ loop.index0 }}].isNull()) {
                            JS_ReportError(cx, "TypeError");
                            return false;
                        }
                    {% endif %}
                    {{jsval2c('args['~ loop.index0 ~']', arg.idlType.idlType, 'inArg_' ~ loop.index0)}} 
                {% endfor %}
                /* End of arguments convertion */

                T *ret = T::Constructor({% for i in range(0, op.arguments.length) %}inArg_{{i}}{{ ', ' if not loop.last }}{%endfor%});

                if (!ret) {
                    JS_ReportError(cx, "TypeError");
                    return false;
                }

                JS::RootedObject rthis(cx, JS_NewObjectForConstructor(cx, &{{ className }}_class, args));

                JS_SetPrivate(rthis, ret);

                args.rval().setObjectOrNull(rthis);
                
                break;
            }
        {% endfor %}
        default:
            JS_ReportError(cx, "TypeError: wrong number of arguments");
            return false;
            break;
    }

    return true;
}

{% endif %}

{% for attrName, attrData in operations %}
bool NativeBindingInterface_{{name}}::js_{{attrName}}(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx, JS_THIS_OBJECT(cx, vp));

    if (!caller) {
        JS_ReportError(cx, "Illegal invocation");
        return false;
    }

    NativeBindingInterface_{{name}} *obj = (NativeBindingInterface_{{name}} *)JS_GetInstancePrivate(cx, caller, &{{ className }}_class, NULL);
    if (!obj) {
        JS_ReportError(cx, "Illegal invocation");
        return false;
    }

    unsigned argcMin = (({{attrData.maxArgs}} > argc) ? (argc) : ({{attrData.maxArgs}}));

    switch (argcMin) {
        {% for op in attrData.lst %}
        case {{op.arguments.length}}:
        {
            /* Start arguments convertion */

            {% for arg in op.arguments %}
                /* Handle argument #{{ loop.index0 }} of type "{{ arg.idlType.idlType }}" */
                {% if not arg.idlType.nullable %}
                    if (args[{{ loop.index0 }}].isNull()) {
                        JS_ReportError(cx, "TypeError");
                        return false;
                    }
                {% endif %}
                {{jsval2c('args['~ loop.index0 ~']', arg.idlType.idlType, 'inArg_' ~ loop.index0)}} 
            {% endfor %}
            /* End of arguments convertion */

            {%if op.idlType.idlType != 'void'%}
                {{ op.idlType.idlType|ctype }} _opret =
            {%endif%}
            obj->{{attrName}}({% for i in range(0, op.arguments.length) %}inArg_{{i}}{{ ', ' if not loop.last }}{%endfor%});

            args.rval().set{{ op.idlType.idlType|jsvaltype|capitalize }}(_opret);

            break;
        }
        {% endfor %}
        default:
            JS_ReportError(cx, "TypeError: wrong number of arguments");
            return false;
            break;
    }

    return true;
}
{% endfor %}

template <typename T>
bool NativeBindingInterface_{{name}}::registerObject(JSContext *cx,
    JS::HandleObject exports)
{
    {% if ctor %}
        JS::RootedObject to(cx);

        to = exports ? exports : JS::CurrentGlobalOrNull(cx);

        JS_InitClass(cx, to, JS::NullPtr(), &{{ className }}_class,
            NativeBindingInterface_{{name}}::js_{{name}}_Constructor<T>,
            0, NULL, {{ className }}_funcs, NULL, NULL);
    {% endif %}
    return true;
}
