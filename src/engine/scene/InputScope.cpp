#include <kerf/engine/scene/InputScope.h>
#include <kerf/engine/node/Node.h>


namespace kerf {

void InputScope::add(const Node* node)
{
    if (node)
    {
        roots.insert(node);
    }
}

void InputScope::remove(const Node* node)
{
    roots.erase(node);
}

void InputScope::clear()
{
    roots.clear();
}

bool InputScope::contains(const Node* node) const
{
    for (const Node* current = node; current != nullptr; current = current->getParent())
    {
        if (roots.contains(current))
        {
            return true;
        }
    }
    return false;
}

} // namespace kerf
