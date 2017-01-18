/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

{% import 'defs.tpl' as defs %}

#include <Binding/ClassMapper.h>
#include "{{ className }}.h"

namespace Nidium {
namespace Binding {

// {{ '{{{' }} JSBinding

{% if ctor %}
    // {{ '{{{' }} Start Constructor
    {{ className }} *{{ className }}::Constructor(JSContext *cx, JS::CallArgs &args,
        JS::HandleObject obj)
    {
        unsigned argc = args.length();
        unsigned argcMin = (({{ constructors['maxArgs'] }} > argc) ? (argc) : ({{ constructors['maxArgs'] }}));

        switch (argcMin) {
            {% for op in constructors.lst %}
                case {{ op.arguments|length }}:
                {
                    /* Start arguments conversion */
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
                    /* End of arguments conversion */

                    {{ className }} *n_{{ className }} = new {{ className }}(
                        {% for i in range(0, op.arguments|length) %}
                            inArg_{{ i }}{{ ' ' if loop.last else ', ' }}
                        {% endfor %}
                    );
                    n_{{ className }}->root();

                    return n_{{ className }};
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

{% set static_methods = getMethods(operations, true) %}
{% set normal_methods = getMethods(operations, false ) %}

{% if normal_methods|length > 0 %}
    // {{ '{{{'  }} Start Operations
    JSFunctionSpec * {{ className }}::ListMethods()
    {
        static JSFunctionSpec funcs[] = {
        {% for attrName, attr in normal_methods %}
            {% if hasAttr(attr.extAttrs, 'Alias') %}
                CLASSMAPPER_FN_ALIAS({{ className }}, {{ attrName }}, {{ attr['maxArgs'] }}, {{ hasAttr( attr.extAttrs, 'Alias') }}),
            {% endif %}
            CLASSMAPPER_FN({{ className }}, {{ attrName }}, {{ attr['maxArgs'] }} ),
        {% endfor %}

        JS_FS_END
        };

        return funcs;
    }
    {% endif %}

{% if static_methods|length > 0 %}
    // {{ '{{{'  }} Start Operations
    JSFunctionSpec * {{ className }}::ListStaticMethods()
    {
        static JSFunctionSpec funcs[] = {
        {% for attrName, attr in static_methods %}
            {% if hasAttr(attr.extAttrs, 'Alias') %}
                CLASSMAPPER_FN_ALIAS({{ className }}, {{ attrName }}, {{ attr['maxArgs'] }}, {{ hasAttr(attr.extAttrs, 'Alias') }}),
            {% endif %}
            CLASSMAPPER_FN({{ className }}, {{ attrName }}, {{ attr['maxArgs'] }} ),
        {% endfor %}

        JS_FS_END
        };

        return funcs;
    }


    {% for attrName, attr in operations %}
        bool {{ className }}::JS_{{ attrName }}(JSContext *cx, JS::CallArgs &args)
        {
            unsigned argc = args.length();
            unsigned argcMin = (({{ attr['maxArgs'] }} > argc) ? (argc) : ({{ attr['maxArgs'] }}));
            {#TODO: optimize when argcMin == 0 #}
            switch (argcMin) {
                {% for op in attr.lst %}
                    case {{ op.arguments|length }}:
                    {
                        /* Start arguments conversion */
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

                        /* End of arguments conversion */
                        {% if op.return.idlType.idlType|ctype != 'void' %}
                            {{ op.return.idlType.idlType|ctype }} _opret =
                        {%endif %}
                        this->{{ attrName }}(
                        {% for i in range(0, op.arguments|length) %}
                            inArg_{{ i }}{{ ' ' if loop.last else ', ' }}
                        {% endfor %}
                        );
                        {% if op.return.idlType.idlType != 'void' %}
                           args.rval().set{{ op.return.idlType.idlType|jsvaltype|capitalize }}(_opret);
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
    {% endfor %}

    // {{ '}}}' }} End Operations
    {% endif %}

{% if members.length > 0 %}
    // {{ '{{{' }} Start Members
    JSPropertySpec *{{ className }}::ListProperties()
    {
        static JSPropertySpec props[] = {
        {% for attr in members %}
            {# TODO: ONLY-SETTER #}
            {% if not attr.readonly %}
                CLASSMAPPER_PROP_GS({{ className }}, {{ attr.name }}),
            {% else %}
                CLASSMAPPER_PROP_G({{ className }}, {{ attr.name }}),
            {% endif %}
        {% endfor %}
            JS_PS_END
        };

        return props;
    }


    {% for attr in members %}
        {% if not attr.readonly %}
            bool {{ className }}::JSSetter_{{ attr.name }}(JSContext *cx, JS::MutableHandleValue vp)
            {
                {# TODO: uniontype#}
                {{ attr.idlType }}
                {{ defs.jsval2c('vp', attr.idlType.idlType, 'inArg_0') }}

                return this->set_{{ attr.name }}(inArg_0);
            }
        {% endif %}

        bool {{ className }}::JSGetter_{{ attr.name }}(JSContext *cx, JS::MutableHandleValue vp)
        {
                {% set need = attr.idlType.idlType %}

                {% if need == 'cstring' %}
                    JS::RootedString jstr(cx, JS_NewStringCopyZ(cx, this->get_{{ attr.name }}()));
                    vp.setString(jstr);
                {% elif attr.idlType.idlType == 'UNKNOWN' %}
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
    {% endfor %}

// {{ '}}}' }} End Members
{% endif %}

{% if hasAttr(extAttrs, 'exposed') %}
    void {{ className }}::RegisterObject(JSContext *cx)
    {
        {% if exposed == 'class' %}
             {{ className }}::ExposeClass<{{ constructors['maxArgs'] }}>(cx, "{{ name }}");
             {# TODO: HAS_RESERVED_SLOTS #}
        {% elif exposed == 'module' %}
            JSModules::RegisterEmbedded("{{ className }}", {{ className }}::RegisterModule);
        {% endif %}
    }

    {% if exposed == 'module' %}
        JSObject *{{ className }}::RegisterModule(JSContext *cx)
        {
            JS::RootedObject exports(cx, {{ className }}::ExposeObject(cx, "{{ name }}"));

            return exports;
        }
    {% endif %}

{% endif %}
// {{ '}}}' }}

} // namespace Binding
} // namespace Nidium

