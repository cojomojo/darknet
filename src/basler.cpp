#ifdef PYLON

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/video.hpp>
#include "opencv2/highgui/highgui_c.h"
#include "opencv2/imgproc/imgproc_c.h"
#include <pylon/PylonIncludes.h>
#include <pylon/usb/BaslerUsbInstantCamera.h>

#ifdef __cplusplus
extern "C" {
#endif

using namespace Pylon;

void capture_from_baslercam(IplImage *basler_img)
{
    PylonAutoInitTerm autoInitTerm;

    try
    {
        CBaslerUsbInstantCamera camera(CTlFactory::GetInstance().CreateFirstDevice());
        std::cout << "Using device " << camera.GetDeviceInfo().GetModelName() << std::endl;
	camera.RegisterConfiguration(new CAcquireContinuousConfiguration, RegistrationMode_ReplaceAll, Cleanup_Delete);

        // Get a camera nodemap in order to access camera parameters
        GenApi::INodeMap& nodemap = camera.GetNodeMap();
        // Open the camera before accessing any parameters
        camera.Open();
        // Create pointers to access the camera Width and Heigh parameters
        GenApi::CIntegerPtr width = nodemap.GetNode("Width");
        GenApi::CIntegerPtr height = nodemap.GetNode("Height");
        // Parameter to control the count of buffers allocated for grabbing. The default is 10.
        camera.MaxNumBuffer = 5;

        CImageFormatConverter formatConverter;
        formatConverter.OutputPixelFormat = PixelType_BGR8packed;

        CPylonImage pylonImage;
        CGrabResultPtr ptrGrabResult;
        int grabbedImages = 0;

        camera.StartGrabbing(1, GrabStrategy_LatestImageOnly);

        while (camera.IsGrabbing())
        {
            // Wait for an image and then retrieve it.
            camera.RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);

            if (ptrGrabResult->GrabSucceeded())
            {
                grabbedImages++;

                std::cout << "SizeX: " << ptrGrabResult->GetWidth() << std::endl;

                // Covert the grabbed buffer to a pylon image.
                formatConverter.Convert(pylonImage, ptrGrabResult);

                // Create an OpenCV image from a pylon image.
                (*basler_img) = cv::Mat(ptrGrabResult->GetHeight(), ptrGrabResult->GetWidth(), CV_8UC3, (uint8_t *) pylonImage.GetBuffer());
            }
        }
    }
    catch (const GenericException &e) 
    {
	std::cerr << e.GetDescription() << std::endl;
    }
}

#ifdef __cplusplus
}
#endif

#endif
