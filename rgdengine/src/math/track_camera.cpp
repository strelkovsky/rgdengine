#include "precompiled.h"

#include <rgde/math/track_camera.h>

namespace math
{

    path_camera::path_camera(camera_ptr pCamera): m_fPosition(0)
    {
        setCamera(pCamera);
        base::lmsg << "path_camera::path_camera()";
    }

    path_camera_ptr path_camera::create(camera_ptr pCamera)
    {
        return path_camera_ptr(new path_camera(pCamera));
    }

    void path_camera::goTo(float position)
    {
        m_fPosition = position;
        apply();
    }

    void path_camera::goForward(float delta)
    {
        goTo(m_fPosition+delta);
    }

    void path_camera::activate()
    {
        apply();
    }

    void path_camera::apply()
    {
        if (m_fPosition < 0)
            m_fPosition = 0;
        if (m_fPosition > m_path.getTotalTime())
            m_fPosition = m_path.getTotalTime();

        if (m_pCamera)
        {
            try{
                CTrack::Key key = m_path.getKey(m_fPosition);
                //todo: возможно потребуется ортонормализация векторов [Dub]
                m_pCamera->lookAt(key.m_vEyePt, key.m_vLookatPt, key.m_vUp);
                m_pCamera->activate();
            }
            catch(...){}
        }
    }

}