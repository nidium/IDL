/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <Core/Utils.h>
#include <Binding/JSUtils.h>
{% import "defs.tpl" as defs %}

#ifndef binding_{{ prefix }}{{ className }}_h__
#define binding_{{ prefix }}{{ className }}_h__

{% set header = hasAttr(extAttrs, 'headerfile') %}
{% if header %}
#include "{{ header.rhs.value }}.h"
{% endif %}

#include "Binding/ClassMapper.h"

namespace Nidium {
namespace Binding {

// {{ '{{{' }} fake {{ className }}

/*
//This is a model of our imaginary base class '{{ className }}'

class {{ className }}_Base
{
public:
    {% if ctor %}
        // Constructor
        {% for constructor in constructors.lst %}
            {{ className }}_Base({{ defs.arglst(constructor.arguments) }});
        {% endfor %}
    {% else %}
        {{ className }}_Base(){};
    {% endif %}
    ~{{ className }}_Base();
protected:
    {% if operations|length > 0 %}
         // Methods
        {% for attrName, attr in operations %}
             {% for op in attr.lst %}
                 {{ op.return_type.idlType|ctype }} {{ op.name }}({{ defs.arglst(op.arguments) }});
             {% endfor %}
        {% endfor %}
    {% endif %}
    {% if members.length > 0 %}
        // Properties
        {# TODO Setter only #}
        {% for attr in members %}
            {% if not attr.readonly %}
                bool set_{{ attr.name }}({{ attr.idlType.idlType|ctype }} {{attr.name }});
            {% endif %}
            {% if attr.idlType.idlType|ctype %}
                {{ attr.idlType.idlType|ctype }} get_{{ attr.name }}();
            {% else %}
                {{ attr.name }} get_{{ attr.name }}();
            {% endif %}
        {% endfor %}
    {% endif %}
private:
    // Properties
    {% for attr in members %}
        {% if attr.idlType.idlType != 'UNKNOWN' %}
            {{ attr.idlType.idlType|ctype }} {{ attr.name }};
        {% else %}
            {{ attr.name }} {{ attr.name }};
        {% endif %}
    {% endfor %}
};
// {{ '}}}' }}
*/

class {{ prefix }}{{ className }} : public ClassMapper{% if hasAttr(extAttrs, 'Events') %}
                                             WithEvents
                                             {% endif %}<{{ prefix }}{{ className }}>, public {{ className }}_Base
{
public:
{% if ctor %}
    {% for constructor in constructors.lst %}
             {{ prefix }}{{ className }}( {{ defs.arglst(constructor.arguments) }})
                : {{ className }}_Base( {{ defs.argcall(constructor.arguments) }} ) { }
       {% else %}
            {{ className }}()::{{className}}_Base(){}
   {% endfor %}
{% endif %}

    ~{{ prefix }}{{ className }}();
    {% if ctor %}
        static {{ prefix }}{{ className }} * Constructor(JSContext *cx, JS::CallArgs &args,
            JS::HandleObject obj);
        static void RegisterObject(JSContext *cx);
    {% endif %}
    {% set static_methods = getMethods(operations, true) %}
    {% set normal_methods = getMethods(operations, false) %}
    {% if normal_methods.lenght > 0 %}
        static JSFunctionSpec * ListMethods();
    {% endif %}
    {% if static_methods.lenght > 0 %}
        static JSFunctionSpec * ListStaticMethods();
    {% endif %}
    {% if members.length > 0 %}
        static JSPropertySpec *ListProperties();
    {% endif %}
    {% if hasAttr(extAttrs, 'exposed') %}
        static void RegisterObject(JSContext *cx);
    {% endif %}
protected:
    {% for attrName, attr in operations %}
        NIDIUM_DECL_JSCALL{%if attr.static%}STATIC{% endif%}({{ attrName }});
    {% endfor %}
    {# TODO ONLY-SETTER #}
    {% for attr in members %}
        {% if not attr.readonly %}
            NIDIUM_DECL_JSGETTERSETTER({{ attr.name }});
        {% else %}
            NIDIUM_DECL_JSGETTER({{ attr.name }});
        {% endif %}
    {% endfor %}
    {% if hasAttr(extAttrs, 'exposed') == 'module' %}
        static JSObject *RegisterModule(JSContext *cx);
    {% endif %}

private:
};
} // namespace Binding
} // namespace Nidium

#endif
