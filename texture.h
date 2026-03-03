#ifndef TEXTURE_H
#define TEXTURE_H

#include "imgui_impl_win32.h"
#ifdef WIN32
#include <d3d9.h>
#include "imgui_impl_dx9.h"
#else
#include <GL/glew.h>      
#include <GLFW/glfw3.h>
#include "imgui_impl_opengl3.h"
#endif

#ifdef WIN32
class Dx9Texture {
public:
    LPDIRECT3DTEXTURE9 tex = nullptr;

public:
    Dx9Texture() = default;
    explicit Dx9Texture(LPDIRECT3DTEXTURE9 t) : tex(t) {}

   /** 
   * @brief deleting copy
   */
    Dx9Texture(const Dx9Texture&) = delete;
    Dx9Texture& operator=(const Dx9Texture&) = delete;

    Dx9Texture(Dx9Texture&& other) noexcept
        : tex(other.tex) {
        other.tex = nullptr;
    }
    Dx9Texture& operator=(Dx9Texture&& other) noexcept {
        if (this != &other) {
            Release();
            tex = other.tex;
            other.tex = nullptr;
        }
        return *this;
    }
    ~Dx9Texture() noexcept {
        Release();
    }
    void Release() noexcept {
        if (tex) {
            tex->Release();
            tex = nullptr;
        }
    }
    void Show(ImVec2 size) const {
        if (tex) {
            ImGui::Image((void*)tex, size);
        }
    }
    bool LoadFromFile(LPDIRECT3DDEVICE9 device, const char* filename) {
        Release();

        int w, h, c;
        unsigned char* data = stbi_load(filename, &w, &h, &c, 4);
        if (!data)
            return false;

        LPDIRECT3DTEXTURE9 raw = nullptr;
        HRESULT hr = device->CreateTexture(
            w, h, 1, 0,
            D3DFMT_A8R8G8B8,
            D3DPOOL_MANAGED,
            &raw, nullptr);

        if (FAILED(hr)) {
            stbi_image_free(data);
            return false;
        }

        D3DLOCKED_RECT rect;
        raw->LockRect(0, &rect, nullptr, 0);

        for (int y = 0; y < h; ++y) {
            memcpy(
                (BYTE*)rect.pBits + y * rect.Pitch,
                data + y * w * 4,
                w * 4
            );
        }
        raw->UnlockRect(0);
        stbi_image_free(data);

        tex = raw;
        return true;
    }
    LPDIRECT3DTEXTURE9 Get() const noexcept {
        return tex;
    }
    explicit operator bool() const noexcept {
        return tex != nullptr;
    }
};
#else
class GLTexture {
    GLuint tex = 0;
public:
    GLTexture() = default;
    ~GLTexture() { if (tex) glDeleteTextures(1, &tex); }

    bool LoadFromFile(const char* filename) {
        int w, h, c;
        unsigned char* data = stbi_load(filename, &w, &h, &c, 4);
        if (!data) return false;

        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(data);
        return true;
    }

    void Show(ImVec2 size) const {
        if (tex) ImGui::Image((void*)(intptr_t)tex, size);
    }

    explicit operator bool() const noexcept { return tex != 0; }
};
#endif
#endif