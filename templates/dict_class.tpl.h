#pragma once

#include <Dict.h>

namespace Nidium {
namespace Binding {

{% raw %}// {{{ {% endraw %} Dict_{{name}}
class Dict_{{name}} : public Dict
{
public:
    Dict_{{name}}() {
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
        const {{type|ctype}} {{attr.name}}() const {
            return m_{{attr.name}};
        }
    {% endfor %}

private:
    {% for attr in members %}
        {% set type = attr.idlType.idlType %}
        {{ type|ctype }} m_{{attr.name}};
    {% endfor %}
};
{% raw %}// }}} {% endraw %}

} // namespace Binding
} // namespace Nidium
