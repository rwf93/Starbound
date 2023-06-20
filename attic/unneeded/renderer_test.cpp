#include "StarMainApplication.hpp"

using namespace Star;

class MainApplication : public Application {
protected:
  void renderInit(RendererPtr renderer) override {
    Application::renderInit(renderer);

    if (renderer->rendererId() == "OpenGL20") {
      renderer->setEffectConfig(Json::parseJson(R"JSON(
          {
            "effectParameters" : {
              "lightMapEnabled" : {
                "type" : "bool",
                "default" : false,
                "uniform" : "lightMapEnabled"
              },
              "lightMapScale" : {
                "type" : "vec2",
                "default" : [1, 1],
                "uniform" : "lightMapScale"
              },
              "lightMapMultiplier" : {
                "type" : "float",
                "default" : 1.0,
                "uniform" : "lightMapMultiplier"
              }
            },

            "effectTextures" : {
              "lightMap" : {
                "textureUniform" : "lightMap",
                "textureSizeUniform" : "lightMapSize",
                "textureAddressing" : "clamp",
                "textureFiltering" : "linear"
              }
            },

            "vertexShader" : "
              #version 110

              uniform vec2 textureSize;
              uniform vec2 screenSize;
              uniform mat3 vertexTransform;
              uniform vec2 lightMapSize;
              uniform vec2 lightMapScale;

              attribute vec2 vertexPosition;
              attribute vec2 vertexTextureCoordinate;
              attribute vec4 vertexColor;

              varying vec2 fragmentTextureCoordinate;
              varying vec4 fragmentColor;
              varying vec2 fragmentLightMapCoordinate;

              void main() {
                vec2 screenPosition = (vertexTransform * vec3(vertexPosition, 1.0)).xy;

                gl_Position = vec4(screenPosition / screenSize * 2.0 - 1.0, 0.0, 1.0);

                fragmentLightMapCoordinate = (screenPosition / lightMapScale) / lightMapSize;
                fragmentTextureCoordinate = vertexTextureCoordinate / textureSize;
                fragmentColor = vertexColor;
              }
            ",

            "fragmentShader" : "
              #version 110

              uniform sampler2D texture;
              uniform bool lightMapEnabled;
              uniform sampler2D lightMap;
              uniform float lightMapMultiplier;

              varying vec2 fragmentTextureCoordinate;
              varying vec4 fragmentColor;
              varying vec2 fragmentLightMapCoordinate;

              void main() {
                vec4 finalColor = texture2D(texture, fragmentTextureCoordinate) * fragmentColor;
                if (lightMapEnabled)
                  finalColor *= texture2D(lightMap, fragmentLightMapCoordinate) * lightMapMultiplier;
                gl_FragColor = finalColor;
              }
            "
          }
        )JSON"));
    }

    Image texture1Image(10, 10, PixelFormat::RGBA32);
    for (unsigned y = 0; y < 10; ++y) {
      for (unsigned x = 0; x < 10; ++x) {
        if (x < 3 || x > 7)
          texture1Image.set(x, y, Vec3B(0, 0, 0));
        else
          texture1Image.set(x, y, Vec3B(255, 255, 255));
      }
    }
    auto texture1 = renderer->createTexture(texture1Image, TextureAddressing::Clamp, TextureFiltering::Linear);

    Image texture2Image(10, 10, PixelFormat::RGBA32);
    for (unsigned y = 0; y < 10; ++y) {
      for (unsigned x = 0; x < 10; ++x) {
        if (y < 3 || y > 7)
          texture2Image.set(x, y, Vec3B(0, 0, 0));
        else
          texture2Image.set(x, y, Vec3B(255, 255, 255));
      }
    }
    auto texture2 = renderer->createTexture(texture2Image, TextureAddressing::Clamp, TextureFiltering::Linear);

    List<RenderPrimitive> primitives;
    for (size_t y = 0; y < 100; ++y) {
      for (size_t x = 0; x < 100; ++x) {
        primitives.append(RenderQuad{(y % 2 == 0) ? texture1 : texture2,
            RenderVertex{Vec2F(x, y), Vec2F(0, 0), Vec4B(255, 255, 255, 255)},
            RenderVertex{Vec2F(x + 1, y), Vec2F(10, 0), Vec4B(255, 255, 255, 255)},
            RenderVertex{Vec2F(x + 1, y + 1), Vec2F(10, 10), Vec4B(255, 255, 255, 255)},
            RenderVertex{Vec2F(x, y + 1), Vec2F(0, 10), Vec4B(255, 255, 255, 255)}});
      }
    }
    m_renderBuffer = renderer->createRenderBuffer();
    m_renderBuffer->set(primitives);

    Image lightMapImage(50, 50, PixelFormat::RGBA32);
    for (unsigned y = 0; y < 50; ++y) {
      for (unsigned x = 0; x < 50; ++x)
        lightMapImage.set(x, y, Vec3B((x + 1) * 5.2 - 5, (y + 1) * 5.2 - 5, 255));
    }

    renderer->setEffectParameter("lightMapEnabled", true);
    renderer->setEffectTexture("lightMap", lightMapImage);
  }

  void render() override {
    Vec2U screenSize = renderer()->screenSize();
    renderer()->setEffectParameter("lightMapScale", Vec2F(screenSize[0] / 50.0f, screenSize[1] / 50.0f));
    renderer()->setScissorRect(RectI(20, 20, screenSize[0] - 20, screenSize[1] - 20));
    renderer()->renderBuffer(m_renderBuffer, Mat3F::scaling(Vec2F(screenSize[0] / 100.0f, screenSize[1] / 100.0f)));
  }

private:
  RenderBufferPtr m_renderBuffer;
};

STAR_MAIN_APPLICATION(MainApplication);
