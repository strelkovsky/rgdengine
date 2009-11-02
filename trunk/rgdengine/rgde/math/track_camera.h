//track_camera.h
#pragma once

#include "camera_controller.h"
#include "track.h"

namespace math
{
    typedef boost::shared_ptr<class path_camera> path_camera_ptr;

    //���������� ������ ��� "������ �������� �� ��������� ����������"
    class path_camera: public base_camera_controller
    {
        path_camera(camera_ptr pCamera);

    public:
        static path_camera_ptr create(camera_ptr pCamera);

        CTrack m_path;

        //����������
        void  goTo(float position);
        void  goForward(float delta);
        void  goBackward(float delta) {goForward(-delta);}
        float getPosition() {return m_fPosition;}

        void activate();

    private:
        void apply();

        float m_fPosition;
    };
}