#include "slipc.h"
#include <memory>
#include <nan.h>

namespace {
class SlipRock : public Nan::ObjectWrap {
  static NAN_METHOD(New) {
    if (info.Length() != 1) {
      Nan::ThrowTypeError("Must have exactly 1 argument");
      return;
    }
    using JSString = v8::Local<v8::String>;
    static Nan::Persistent<v8::Function> constructor;
    if (info.IsConstructCall()) {
      v8::MaybeLocal<v8::String> s = Nan::To<v8::String>(info[0]);
      if (s.IsEmpty())
        return;
      size_t len = s.ToLocalChecked()->Utf8Length();
      std::unique_ptr<char[]> buf(new char[len + 1]);
      s.ToLocalChecked()->WriteUtf8(buf.get(), len);
      buf[len] = '\0';
      if (strlen(buf.get()) != len) {
        Nan::ThrowSyntaxError("NUL not allowed in a filename");
        return;
      }
      struct SLIPC_Connection *const connection = SLIPC_socket(buf.get(), len);
      if (nullptr == connection) {
        info.GetIsolate()->ThrowException(Nan::ErrnoException(
            errno, "SLIPC_socket", "failed to create socket", buf.get()));
        return;
      }

      info.GetReturnValue().Set(info.This());
    } else {
      const int argc = 1;
      v8::Local<v8::Value> argv[argc] = {info[0]};
      auto cons = Nan::New(constructor);
      info.GetReturnValue().Set(cons->NewInstance(argc, argv));
    }
  }
  struct SLIPC_Connection *con;

public:
  static NAN_MODULE_INIT(Init) {
    auto tpl = Nan::New<v8::FunctionTemplate>(SlipRock::New);
    tpl->SetClassName(Nan::New("SlipRock").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    Nan::SetPrototypeMethod(tpl, "bind", SlipRock::bind);
  }
};

NAN_METHOD(SlipRock) {
  if (info.Length() != 1) {
    Nan::ThrowTypeError("Must have exactly 1 argument");
    return;
  }
}

static void Init(v8::Local<v8::Object> exports) {}
}

NODE_MODULE(hello, Init)
