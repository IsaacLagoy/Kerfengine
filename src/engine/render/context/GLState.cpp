#include "engine/render/context/GLState.h"

#include <stdexcept>


void GLState::save()
{
    // save fbo
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fbo);

    // save viewport
    glGetIntegerv(GL_VIEWPORT, viewport);

    // save depth enabled
    depthWasEnabled = glIsEnabled(GL_DEPTH_TEST);

    saved = true;
}

void GLState::restore() const
{
    // check if we have saved a state
    if (!saved)
    {
        throw std::runtime_error("GLState: cannot load unsaved state!");
    }

    // load fbo
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // load viewport
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

    // set enable 
    if (depthWasEnabled) 
    {
        glEnable(GL_DEPTH_TEST);
    }
    else 
    {
        glDisable(GL_DEPTH_TEST);
    }
}
