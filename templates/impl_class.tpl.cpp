/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

{% import 'defs.tpl' as defs %}

{% set static_methods = getMethods(members, true) %}
{% set normal_methods = getMethods(members, false ) %}
{% set properties = getProperties(members) %}
{% set exposed = hasAttr(extAttrs, 'exposed') %}

#include <Binding/ClassMapper.h>
#include "{{ prefix }}{{ className }}.h"

{% if exposed.rhs.value == 'module' or exposed.rhs.value == 'embed' %}
#include "Binding/JSModules.h"
{% endif %}

namespace Nidium {
namespace Binding {

// {{ '{{{' }} JSBinding

{% if ctor %}
    // {{ '{{{' }} Start Constructor
    {{ prefix }}{{ className }} *{{ prefix }}{{ className }}::Constructor(JSContext *cx, JS::CallArgs &args,
        JS::HandleObject obj)
    {
        unsigned argc = args.length();
        unsigned argcMin = (({{ constructors.maxArgs }} > argc) ? (argc) : ({{ constructors.maxArgs }}));

        switch (argcMin) {
            {% for op in constructors.lst %}
                case {{ op.arguments|length }}:
                {
                    /* Start argument conversion */
                    {% for arg in op.arguments %}
                        /* Handle argument #{{ loop.index0 }} of type "{{ arg.idlType.idlType }}" */
                        {% if not arg.idlType.nullable %}
                            if (args[{{ loop.index0 }}].isNull()) {
                                JS_ReportError(cx, "TypeError");
                                return nullptr;
                            }
                        {% endif %}
                        {{ defs.jsval2c('args[' ~ loop.index0 ~ ']', arg.idlType.idlType, 'inArg_' ~  loop.index0 , 'nullptr') }}
                    {% endfor %}
                    /* End of argument conversion */

                    {{ prefix }}{{ className }} *n_{{ prefix }}{{ className }} = new {{ prefix }}{{ className }}(
                        {% for i in range(0, op.arguments|length) %}
                            inArg_{{ i }}{{ ' ' if loop.last else ', ' }}
                        {% endfor %}
                    );

                    return n_{{ prefix }}{{ className }};
                    break;
                }
            {% endfor %}
            default:
                JS_ReportError(cx, "TypeError: wrong number of arguments");
                return nullptr;
                break;
        }

        return nullptr;
    }
    // {{ '}}}' }} End Constructor
{% endif %}

{% if (normal_methods|length + static_methods|length) > 0 %}
    // {{ '{{{' }} Methods
    {% if normal_methods|length > 0 %}
        // List normal methods
        JSFunctionSpec * {{ prefix }}{{ className }}::ListMethods()
        {
            static JSFunctionSpec funcs[] = {
            {% for attr in normal_methods %}
                {% set maxArgs = operations[attr.name].maxArgs %}
                {% if hasAttr(attr.extAttrs, 'Alias') %}
                    CLASSMAPPER_FN_ALIAS({{ prefix }}{{ className }}, {{ attr.name }}, {{ maxArgs }}, {{ hasAttr( attr.extAttrs, 'Alias') }}),
                {% endif %}
                CLASSMAPPER_FN({{ prefix }}{{ className }}, {{ attr.name }}, {{ maxArgs }} ),
            {% endfor %}

            JS_FS_END
            };

            return funcs;
        }
    {% endif %}

    {% if static_methods|length > 0 %}
        // List static methods
        JSFunctionSpec * {{ prefix }}{{ className }}::ListStaticMethods()
        {
            static JSFunctionSpec funcs[] = {
            {% for attr in static_methods %}
                {% set maxArgs = operations[attr.name].maxArgs %}
                {% if hasAttr(attr.extAttrs, 'Alias') %}
                    CLASSMAPPER_FN_ALIAS({{ prefix }}{{ className }}, {{ attr.name }}, {{ maxArgs }}, {{ hasAttr(attr.extAttrs, 'Alias') }}),
                {% endif %}
                {% if attr.static %}
                    CLASSMAPPER_FN_STATIC({{ prefix }}{{ className }}, {{ attr.name }}, {{ maxArgs }} ),
                {% else %}
                    CLASSMAPPER_FN({{ prefix }}{{ className }}, {{ attr.name }}, {{ maxArgs }} ),
                {% endif %}
            {% endfor %}

            JS_FS_END
            };

            return funcs;
        }
    {% endif %}

    // Methods implementation
    {% for method_list in [normal_methods, static_methods ] %}
        {% for attr in method_list %}
            {% set maxArgs = operations[attr.name].maxArgs %}
            // {{ '{{{' }} Start method {{ attr.name }}
            {% if attr.static %}
                bool {{ prefix }}{{ className }}::JSStatic_{{ attr.name }}(JSContext *cx, JS::CallArgs &args)
            {% else %}
                bool {{ prefix }}{{ className }}::JS_{{ attr.name }}(JSContext *cx, JS::CallArgs &args)
            {%endif %}
            {
                unsigned argc = args.length();
                unsigned argcMin = (({{maxArgs }} > argc) ? (argc) : ({{ maxArgs }}));
                {#TODO: optimize when argcMin == 0 #}
                switch (argcMin) {
                    {% for op in operations[attr.name].lst %}
                        case {{ op.arguments|length }}:
                        {
                            /* Start argument conversion */
                            {% for arg in op.arguments %}
                                /* Handle argument #{{ loop.index0 }} of type "{{ arg.idlType.idlType }}" */
                                {% if not arg.idlType.nullable %}
                                    if (args[{{ loop.index0 }}].isNull()) {
                                        JS_ReportError(cx, "TypeError");
                                        return false;
                                    }
                                {% endif %}
                                {# TODO union-types  #}
                                {% if arg.idlType.idlType  != 'UNKNOWN' %}
                                    {{ defs.jsval2c('args[' ~ loop.index0 ~ ']', arg.idlType.idlType, 'inArg_' ~ loop.index0) }}
                                {% else %}
                                    {{ defs.jsval2c('args[' ~ loop.index0 ~ ']', arg.type, 'inArg_' ~ loop.index0) }}
                                {% endif %}
                            {% endfor %}

                            /* End of argument conversion */
                            {# if op.idlType.idlType|ctype != 'void' #}
                                {{ op.idlType.idlType|ctype }} _opret =
                            {#endif #}
                            {% if attr.static %}{{ prefix }}{{ className }}::{% else %}this->{% endif %}
                            {{ attr.name }}(
                            {% for i in range(0, op.arguments|length) %}
                                inArg_{{ i }}{{ ' ' if loop.last else ', ' }}
                            {% endfor %}
                            );
                            {% set need = attr.idlType.idlType %}
                            {% if need == 'cstring' %}
                                JS::RootedString jstr0(cx, JS_NewStringCopyZ(cx, _opret));
                                args.rval().setString(jstr);
                            {% elif need == 'boolean' %}
                                args.rval().setBoolean(_opret);
                            {% elif attr.idlType.idlType == 'unknown' %}
                                {# TODO InterfaceType {{ need }}  {{ need.idlType}} #}
                            {% else %}
                                JS::RootedValue jval(cx);
                                if (!JS::{{ need | convert }}(cx, jval, &_opret)) {
                                    JS_ReportError(cx, "TypeError");
                                    return false;
                                }
                               args.rval().set(jval);
                            {% endif %}

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
            // {{ '}}}' }} End method {{ attr.name }}
        {% endfor %}
    {% endfor %}

    // {{ '}}}' }} End Methods
    {% endif %}

{% if properties|length > 0 %}
    // {{ '{{{' }} Properties
    // List properties
    JSPropertySpec *{{ prefix }}{{ className }}::ListProperties()
    {
        static JSPropertySpec props[] = {
        {% for attr in properties %}
            {# TODO: ONLY-SETTER #}
            {% if not attr.readonly %}
                CLASSMAPPER_PROP_GS({{ prefix }}{{ className }}, {{ attr.name }}),
            {% else %}
                CLASSMAPPER_PROP_G({{ prefix }}{{ className }}, {{ attr.name }}),
            {% endif %}
        {% endfor %}
            JS_PS_END
        };

        return props;
    }

    {% for attr in properties %}
        // {{ '{{{' }} Start property {{ attr.name }}
        {% if not attr.readonly %}
            // Setter {{ attr.name }}
            bool {{ prefix }}{{ className }}::JSSetter_{{ attr.name }}(JSContext *cx, JS::MutableHandleValue vp)
            {
                {# TODO: uniontype attr.idlType #}
                {{ defs.jsval2c('vp', attr.idlType.idlType, 'inArg_0') }}

                return this->set_{{ attr.name }}(inArg_0);
            }
        {% endif %}
        // Getter {{ attr.name }}
        bool {{ prefix }}{{ className }}::JSGetter_{{ attr.name }}(JSContext *cx, JS::MutableHandleValue vp)
        {
                {% set need = attr.idlType.idlType %}

                {% if need == 'cstring' %}
                    JS::RootedString jstr(cx, JS_NewStringCopyZ(cx, this->get_{{ attr.name }}()));
                    vp.setString(jstr);
                {% elif need == 'boolean' %}
                    {{ need|ctype }} cval = this->get_{{ attr.name }}();
                    vp.setBoolean(cval);
                {% elif attr.idlType.idlType == 'unknown' %}
                    {# TODO InterfaceType {{ need.name }}  {{attr.type.__class__}} #}
                {% else %}
                    {{ need|ctype }} cval = this->get_{{ attr.name }}();
                    JS::RootedValue jval(cx);
                    if (!JS::{{ need | convert }}(cx, jval, &cval)) {
                        JS_ReportError(cx, "TypeError");
                        return false;
                    }
                   vp.set(jval);
                {% endif %}

            return true;
        }
        // {{ '}}}' }} End property {{ attr.name }}
    {% endfor %}

// {{ '}}}' }} End Properties
{% endif %}

{% if exposed %}
// {{ '{{{' }} Registration
    {% if exposed.rhs.value == 'class' %}
        {# nothing special #}
    {% elif exposed.rhs.value == 'embed' %}
        static JSObject *registerCallback(JSContext *cx)
        {
            JS::RootedObject obj(cx, JS_NewPlainObject(cx));
            {{ prefix }}{{ className }}::ExposeClass<1>(cx, "{{ className }}", 0, {{ prefix }}{{ className }}::kEmpty_ExposeFlag, obj);
            JS::RootedValue val(cx);
            if (!JS_GetProperty(cx, obj, "{{ className }}", &val)) {
                return nullptr;
            }
            JS::RootedObject ret(cx, val.toObjectOrNull());

            return ret;
        }
    {% elif exposed.rhs.value == 'module' %}
        JSObject *{{ prefix }}{{ className }}::RegisterModule(JSContext *cx)
        {
            JS::RootedObject exports(cx, {{ prefix }}{{ className }}::ExposeObject(cx, "{{ name }}"));

            return exports;
        }
    {% else %}
        {# thus this exposed == 'once', the rhs.value is the name to expose to #}
    {% endif %}

    void {{ prefix }}{{ className }}::RegisterObject(JSContext *cx)
    {
        {# TODO: HAS_RESERVED_SLOTS #}
        {% if exposed.rhs.value == 'class' %}
        {% elif exposed.rhs.value == 'embed' %}
             JSModules::RegisterEmbedded("{{ className }}", registerCallback);
        {% elif exposed.rhs.value == 'module' %}
            JSModules::RegisterEmbedded("{{ className }}", {{ prefix }}{{ className }}::RegisterModule);
        {% else %}
             {# thus this exposed == 'once', the rhs.value is the name to expose to #}
             {{ prefix }}{{ className }}::ExposeClass(cx, "{{ className }}");
             {{ prefix }}{{ className }}::CreateUniqueInstance(cx, new {{ prefix}}{{ className }}(), "{{ exposed.rhs.value }}");
        {% endif %}
        {# //todo: call the created instances->registrationHook();#}
    }
{% endif %}
// {{ '}}}' }}

} // namespace Binding
} // namespace Nidium

