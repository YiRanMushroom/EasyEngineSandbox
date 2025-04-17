module;

export module main;

import Easy.Core.Basic;
import Easy.Core.Engine;
import Easy.Core.Context;
import Easy.Platform.OpenGLEngineContext;

int main() {
    auto engine = Easy::Engine::FromContext(std::make_shared<Easy::OpenGLEngineContext>());
    engine->Run();
    return 0;
}