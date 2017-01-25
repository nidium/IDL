/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <Core/Utils.h>
#include <Binding/JSUtils.h>
{% import "defs.tpl" as defs %}

{% set static_methods = getMethods(members, true) %}
{% set normal_methods = getMethods(members, false) %}
{% set properties = getProperties(members) %}

#ifndef binding_{{ prefix }}{{ className }}_h__
#define binding_{{ prefix }}{{ className }}_h__

{% set header = hasAttr(extAttrs, 'headerfile') %}
{% if header %}
#include "{{ header.rhs.value }}.h"
{% endif %}

#include "Binding/ClassMapper.h"

namespace Nidium {
namespace Binding {


// {{ '{{{' }} fake {{ className }}_Base
#if 0
//This is a model of our imaginary base class '{{ className }}_Base'

class {{ className }}_Base
{
public:
    // Constructor
    {% if ctor %}
        {% for constructor in constructors.lst %}
            {{ className }}_Base({{ defs.arglst(constructor.arguments) }});
        {% endfor %}
    {% else %}
        {{ className }}_Base(){};
    {% endif %}
    ~{{ className }}_Base();
protected:
    // Methods
    {% for method_list in [normal_methods, static_methods ] %}
        {% for op in method_list %}
             {% if operations[op.name].lst|length > 1 %}
                 // Overloading {{ op.name}}-s methods
             {% endif %}
             {% for attr in operations[op.name].lst %}
                {% if attr.static %}static {% endif %}
                {# if op.idlType.idlType|ctype != 'void' #}
                    {{ op.idlType.idlType|ctype }}
                {#endif #}
                {{ attr.return_type.idlType|ctype }} {{ attr.name }}({{ defs.arglst(attr.arguments) }});
             {% endfor %}
        {% endfor %}
    {% endfor %}
    // Properties
    {% if properties|length > 0 %}
        {# TODO Setter only #}
        {% for attr in properties %}
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
    {% for attr in properties %}
        {% if attr.idlType.idlType != 'UNKNOWN' %}
            {{ attr.idlType.idlType|ctype }} {{ attr.name }};
        {% else %}
            {{ attr.name }} {{ attr.name }};
        {% endif %}
    {% endfor %}
};
#endif
// {{ '}}}' }}

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
        //Constructor
        static {{ prefix }}{{ className }} * Constructor(JSContext *cx, JS::CallArgs &args,
            JS::HandleObject obj);
    {% endif %}
    //Listings
    {% if normal_methods|length > 0 %}
        static JSFunctionSpec * ListMethods();
    {% endif %}
    {% if static_methods|length > 0 %}
        static JSFunctionSpec * ListStaticMethods();
    {% endif %}
    {% if properties|length > 0 %}
        static JSPropertySpec * ListProperties();
    {% endif %}
    //Registration
    {% if hasAttr(extAttrs, 'exposed') %}
        static void RegisterObject(JSContext *cx);
    {% endif %}
protected:
    //Methods
    {% for attr in static_methods %}
        {# this is one js entry point, no method overloading #}
        NIDIUM_DECL_JSCALL_STATIC({{ attr.name }});
    {% endfor %}
    {% for attr in normal_methods %}
        {# this is one js entry point, no method overloading #}
        NIDIUM_DECL_JSCALL({{ attr.name }});
    {% endfor %}
    {# TODO ONLY-SETTER #}
    //Properties
    {% for attr in properties %}
        {% if not attr.readonly %}
            NIDIUM_DECL_JSGETTERSETTER({{ attr.name }});
        {% else %}
            NIDIUM_DECL_JSGETTER({{ attr.name }});
        {% endif %}
    {% endfor %}
    //Registration
    {% if hasAttr(extAttrs, 'exposed') == 'module' %}
        static JSObject *RegisterModule(JSContext *cx);
    {% endif %}

private:
};
} // namespace Binding
} // namespace Nidium

#endif
