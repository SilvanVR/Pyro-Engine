#define NOMINMAX

#include <nan.h>
#include <vector>
#include <iostream>

#include "vulkan-core/rendering_engine_interface.hpp"
#include "json scene/json_scene_manager.h"
#include "threading/thread_pool.hpp"

#define WIDTH   1280
#define HEIGHT  720

Pyro::RenderingEngine*      renderer;

std::mutex                  rendererMutex;      // Synchronizes access to the renderer
std::condition_variable     shutdownCondVar;    // Checks if the renderer can be destroyed
uint32_t                    numJobs = 0;        // Counts requests

const std::string resFolder = "../../vulkan-rendering-engine/vulkan-rendering-engine/res/";

NAN_METHOD(init)
{
    using namespace Pyro;

    std::string res = resFolder;
    if(info.Length() > 0)
    {
        v8::String::Utf8Value val(info[0]->ToString()); 
        res = *val;   
    }

    VFS::mount("models", res + "models");
    VFS::mount("textures", res + "textures");
    VFS::mount("shaders", res + "shaders");
    VFS::mount("fonts", res + "fonts");
    VFS::mount("scenes", res + "scenes");

    std::cout << "Init Renderer" << std::endl;
    renderer = new RenderingEngine(Vec2ui(WIDTH,HEIGHT));     
}


class ShutdownRenderer : public Nan::AsyncWorker 
{
    public:
        ShutdownRenderer(Nan::Callback* callback) 
            : Nan::AsyncWorker(callback) {}

        void Execute() {  
            std::unique_lock<std::mutex> lock(rendererMutex);
            shutdownCondVar.wait(lock, []() -> bool { return numJobs == 0; });
            std::cout << "Shutdown Renderer" << std::endl;
            delete renderer;
        }

        void HandleOKCallback () {}
};

// Spawns a thread which will wait until all RenderJobs are done and deletes the engine
NAN_METHOD(shutdown)
{
    Nan::Callback *callback = new Nan::Callback(info[0].As<v8::Function>());
    Nan::AsyncQueueWorker(new ShutdownRenderer(callback));
}

void buffer_delete_callback(char* data, void* the_vector) 
{
    delete reinterpret_cast<std::vector<unsigned char> *> (the_vector);
}


class RenderJob : public Nan::AsyncWorker 
{
    public:
        RenderJob(const std::string& sceneAsJson, Nan::Callback* callback) 
            : Nan::AsyncWorker(callback), json(sceneAsJson)
        {
            numJobs++;   
        }

        ~RenderJob()
        {
            numJobs--;
            shutdownCondVar.notify_one();
        }

        void Execute() {  
            using namespace Pyro;

            // Allow only one thread to use actively the renderer
            rendererMutex.lock();

            // Create the scene and switch to it     
            JSONSceneManager::switchScene(json);

            pixels = new std::vector<unsigned char>();
        
            renderer->draw([&](const ImageData& imageData) {
                *pixels = imageData.pixels; 
                rendererMutex.unlock();
            });
        }

        void HandleOKCallback () {
            v8::Local<v8::Object> pixelData = 
                Nan::NewBuffer((char*)pixels->data(), pixels->size(), buffer_delete_callback, pixels).ToLocalChecked();
            v8::Local<v8::Value> argv[] = { Nan::Null(), pixelData };
            callback->Call(2, argv);
        }

    private:
        std::vector<unsigned char>* pixels;
        std::string json;
};

NAN_METHOD(renderAsync) {
    // First argument is the json which describes the scene
    v8::String::Utf8Value val(info[0]->ToString()); 
    std::string json = *val;   
    
    // Second argument is the callback which should be called when rendering has been finished
    Nan::Callback *callback = new Nan::Callback(info[1].As<v8::Function>());
    Nan::AsyncQueueWorker(new RenderJob(json, callback));
}

NAN_METHOD(setResolution) {
    using namespace Pyro;
    Nan::Maybe<uint32_t> x = Nan::To<uint32_t>(info[0]); 
    Nan::Maybe<uint32_t> y = Nan::To<uint32_t>(info[1]); 
    std::unique_lock<std::mutex> lock(rendererMutex);
    Vec2ui newRes(x.FromJust(),y.FromJust());
    renderer->setFinalResolution(newRes);
}

NAN_MODULE_INIT(Init) {
    Nan::Set(target, Nan::New<v8::String>("init").ToLocalChecked(),
        Nan::GetFunction(Nan::New<v8::FunctionTemplate>(init)).ToLocalChecked());
    Nan::Set(target, Nan::New<v8::String>("setResolution").ToLocalChecked(),
        Nan::GetFunction(Nan::New<v8::FunctionTemplate>(setResolution)).ToLocalChecked());
    Nan::Set(target, Nan::New<v8::String>("shutdown").ToLocalChecked(),
        Nan::GetFunction(Nan::New<v8::FunctionTemplate>(shutdown)).ToLocalChecked());
    Nan::Set(target, Nan::New<v8::String>("renderAsync").ToLocalChecked(),
        Nan::GetFunction(Nan::New<v8::FunctionTemplate>(renderAsync)).ToLocalChecked());
}

NODE_MODULE(renderer, Init)
