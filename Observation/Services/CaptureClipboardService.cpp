#include "CaptureClipboardService.h"
#include <fstream>

#if defined(__linux__)
#include <unistd.h>
#endif

CaptureClipboardService& CaptureClipboardService::getInstance() {
    static CaptureClipboardService inst;
    return inst;
}

ClipboardState CaptureClipboardService::getCurrentState() const {
#if defined(_WIN32) || defined(_WIN64)
    return getCurrentState_WIN();
#elif defined(__linux__)
    return getCurrentState_LINUX();
#else
    static_assert(false, "unsupported platform");
#endif
}

#if defined(_WIN32) || defined(_WIN64)
std::string CaptureClipboardService::utf16ToUtf8(const wchar_t *w, int wlen) {
    if (wlen == 0) return {};
    const int n = ::WideCharToMultiByte(CP_UTF8, 0, w, wlen, nullptr, 0, nullptr, nullptr);
    std::string out(static_cast<std::size_t>(n), '\0');
    ::WideCharToMultiByte(CP_UTF8, 0, w, wlen, out.data(), n, nullptr, nullptr);
    return out;
}

void CaptureClipboardService::captureText_WIN(ClipboardState &state) const {
    HANDLE h = ::GetClipboardData(CF_UNICODETEXT);
    if (!h) return;
    auto* w = static_cast<wchar_t*>(::GlobalLock(h));
    if (!w) return;
    std::string utf8 = utf16ToUtf8(w, static_cast<int>(::wcslen(w)));
    ::GlobalUnlock(h);
    if (!utf8.empty())
        state.addItem(ClipboardItem::makeText(
                ClipboardDataType::Text, std::move(utf8), "CF_UNICODETEXT"));
}

void CaptureClipboardService::captureFiles_WIN(ClipboardState &state) const {
    HANDLE h = ::GetClipboardData(CF_HDROP);
    if (!h) return;
    auto drop = static_cast<HDROP>(::GlobalLock(h));
    if (!drop) return;

    UINT n = ::DragQueryFileW(drop, 0xFFFFFFFF, nullptr, 0);
    std::vector<std::string> files;
    std::size_t total = 0;
    for (UINT i = 0; i < n; ++i) {
        UINT len = ::DragQueryFileW(drop, i, nullptr, 0);
        std::wstring buf(len + 1, L'\0');
        ::DragQueryFileW(drop, i, buf.data(), len + 1);
        buf.resize(len);
        std::string path = utf16ToUtf8(buf.c_str(), static_cast<int>(buf.size()));
        WIN32_FILE_ATTRIBUTE_DATA fa{};
        if (::GetFileAttributesExW(buf.c_str(), GetFileExInfoStandard, &fa))
            total += (static_cast<std::size_t>(fa.nFileSizeHigh) << 32) | fa.nFileSizeLow;
        files.push_back(std::move(path));
    }
    ::GlobalUnlock(h);
    if (!files.empty())
        state.addItem(ClipboardItem::makeFileList(std::move(files), total));
}

void CaptureClipboardService::captureImage_WIN(ClipboardState &state) const {
    HANDLE h = ::GetClipboardData(CF_DIB);
    if (!h) return;
    auto* bytes = static_cast<std::byte*>(::GlobalLock(h));
    SIZE_T size = ::GlobalSize(h);
    if (!bytes || size == 0) { if (bytes) ::GlobalUnlock(h); return; }

    wchar_t dir[MAX_PATH]; ::GetTempPathW(MAX_PATH, dir);
    wchar_t file[MAX_PATH]; ::GetTempFileNameW(dir, L"clp", 0, file);
    std::string path = utf16ToUtf8(file, static_cast<int>(::wcslen(file)));

    std::ofstream out(path, std::ios::binary);
    out.write(reinterpret_cast<const char*>(bytes), static_cast<std::streamsize>(size));
    out.close();
    ::GlobalUnlock(h);

    state.addItem(ClipboardItem::makeDeferred(
            ClipboardDataType::Image, "CF_DIB",
            static_cast<std::size_t>(size), ClipboardItem::makeFileLoader(path)));
}

ClipboardState CaptureClipboardService::getCurrentState_WIN() const {
    ClipboardState state;
    ClipboardGuard clip;
    if (!clip.ok()) return state;

    captureText_WIN(state);
    captureFiles_WIN(state);
    captureImage_WIN(state);
    return state;
}
#endif

#if defined(__linux__)
void CaptureClipboardService::ensureGtk() {
    static bool inited = false;
    if (!inited) {
        gtk_init_check(nullptr, nullptr);
        inited = true;
    }
}

ClipboardState CaptureClipboardService::getCurrentState_LINUX() const {
    ensureGtk();
    ClipboardState state;
    GtkClipboard* clip = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    if (!clip) return state;

    captureText_LINUX(state, clip);
    captureImage_LINUX(state, clip);
    return state;
}

void CaptureClipboardService::captureText_LINUX(ClipboardState& state, GtkClipboard* clip) const {
    if (!gtk_clipboard_wait_is_text_available(clip)) return;
    gchar* txt = gtk_clipboard_wait_for_text(clip);
    if (!txt) return;
    std::string utf8(txt);
    g_free(txt);
    if (!utf8.empty())
        state.addItem(ClipboardItem::makeText(
            ClipboardDataType::Text, std::move(utf8), "text/plain"));
}

void CaptureClipboardService::captureImage_LINUX(ClipboardState& state, GtkClipboard* clip) const {
    if (!gtk_clipboard_wait_is_image_available(clip)) return;
    GdkPixbuf* pix = gtk_clipboard_wait_for_image(clip);
    if (!pix) return;

    gchar* tmpl = g_strdup("/tmp/clpXXXXXX.png");
    int fd = g_mkstemp(tmpl);
    std::string path = tmpl;
    std::size_t size = 0;
    if (fd != -1) {
        close(fd);
        GError* err = nullptr;
        gdk_pixbuf_save(pix, path.c_str(), "png", &err, nullptr);
        if (err) g_error_free(err);
        std::ifstream in(path, std::ios::binary | std::ios::ate);
        if (in) size = static_cast<std::size_t>(in.tellg());
    }
    g_free(tmpl);
    g_object_unref(pix);

    if (size > 0)
        state.addItem(ClipboardItem::makeDeferred(
            ClipboardDataType::Image, "image/png", size, ClipboardItem::makeFileLoader(path)));
}
#endif