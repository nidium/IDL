#pragma once

#include <NativeBindingDict.h>

class NativeBindingDict_{{name}} : public NativeBindingDict
{
public:
    NativeBindingDict_{{name}}() {
        {% for attr in members %}
        {% set type = attr.idlType.idlType %}
        {% if type == 'cstring' %}
        m_{{attr.name}} = {% if not attr.default %}NULL{%else%}strdup("{{ attr.default.value }}"){%endif%};
        {% else %}
        m_{{attr.name}} = {% if not attr.default %}NULL{%else%}{{ attr.default.value }}{%endif%};
        {% endif %}
        {% endfor %}
    }

    /*
        TODO dtor
    */

    bool initWithJSVal(JSContext *cx, JS::HandleValue v)
    {
        if (!v.isObject()) {
            return false;
        }
        JS::RootedObject curobj(cx, &v.toObject());
        JS::RootedValue curopt(cx);
        {% for attr in members %}
        {% set type = attr.idlType.idlType %}

        if (!JS_GetProperty(cx, curobj, "{{ attr.name }}", &curopt)) {
            return false;
        } else {
            {% if type == 'cstring' %}
            JS::RootedString curstr(cx, JS::ToString(cx, curopt));
            JSAutoByteString c_curstr(cx, curstr);

            m_{{attr.name}} = strdup(c_curstr.ptr());
            {% elif type == 'unsigned short' %}
            if (!JS::ToUint16(cx, curopt, &m_{{attr.name}})) {
                return false;
            }
            {% endif %}
        }

        {% endfor %}

        return true;
    }
    {% for attr in members %}
    {% set type = attr.idlType.idlType %}
    {% if type == 'cstring' %}
    const char *{{attr.name}}() const {
        return m_{{attr.name}};
    }

    {% else %}
    {{type}} {{attr.name}}() const {
        return m_{{attr.name}};
    }
    {% endif %}
    {% endfor %}
private:
    {% for attr in members %}
    {% set type = attr.idlType.idlType %}
    {% if type == 'cstring' %}
    char *m_{{attr.name}};
    {% else %}
    {{type}} m_{{attr.name}};
    {% endif %}
    {% endfor %}
};