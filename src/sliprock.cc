#include <memory>
#include <nan.h>
#include <sliprock.h>

namespace {

static Nan::Persistent<v8::Function> &constructor() {
  static Nan::Persistent<v8::Function> mycon;
  return mycon;
}

struct SlipRock : public Nan::ObjectWrap {
  static NAN_METHOD(bind) {
    if (info.Length() != 1) {
      Nan::ThrowTypeError("SlipRock#bind takes 1 argument");
      return;
    }
    if (!info[0]->IsFunction()) {
      Nan::ThrowTypeError("SlipRock#bind: paramenter must be a function");
      return;
    }
    SlipRock *s = ObjectWrap::Unwrap<SlipRock>(info.This());
    s->Ref();
  }
  static NAN_METHOD(NewInstance) { info.GetReturnValue().Set(info.This()); }
  static NAN_METHOD(New);
  struct SliprockConnection *con;

  ~SlipRock() { sliprock_close(this->con); }

  static NAN_MODULE_INIT(Init) {
    auto tpl = Nan::New<v8::FunctionTemplate>(SlipRock::NewInstance);
    tpl->SetClassName(Nan::New("SlipRock").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    Nan::SetPrototypeMethod(tpl, "bind", SlipRock::bind);
    constructor().Reset(Nan::GetFunction(tpl).ToLocalChecked());
    auto q = Nan::New<v8::FunctionTemplate>(SlipRock::New);
    Nan::Set(target, Nan::New("new").ToLocalChecked(),
             Nan::GetFunction(q).ToLocalChecked());
  }
  void Wrap(v8::Local<v8::Object> o) { return this->Nan::ObjectWrap::Wrap(o); }
};

// Worker that queues the object
struct SlipRockNewWorker : public Nan::AsyncWorker {
  SlipRockNewWorker(std::unique_ptr<char[]> buf, size_t length,
                    Nan::Callback *callback, v8::Isolate *isolate)
      : AsyncWorker(callback), callback(callback), isolate(isolate),
        buf(std::move(buf)), size(length), con(nullptr) {}
  ~SlipRockNewWorker() { sliprock_close(con); }

  void Execute() override {
    errno = 0;
    this->con = sliprock_socket(buf.get(), size);
    this->err = errno;
    if (nullptr == this->con) {
      this->SetErrorMessage("Failed to create socket");
    }
  }

  void HandleOKCallback() override {
    assert(nullptr != this->con);
    auto instance =
        Nan::NewInstance(constructor().Get(this->isolate)).ToLocalChecked();
    (new SlipRock())->Wrap(instance);
    con = nullptr;
    v8::Local<v8::Value> args[2] = {
        Nan::Null(), instance,
    };
    callback->Call(2, args);
  }

private:
  Nan::Callback *callback;
  v8::Isolate *isolate;
  std::unique_ptr<char[]> buf;
  size_t size;
  int err;
  SliprockConnection *con;
};
NAN_METHOD(SlipRock::New) {
  if (info.Length() != 2) {
    Nan::ThrowTypeError("Must have exactly 2 arguments");
    return;
  }
  if (info.IsConstructCall()) {
    Nan::ThrowTypeError("SlipRock#new: not a constructor");
    return;
  }
  v8::MaybeLocal<v8::String> s = Nan::To<v8::String>(info[0]);
  if (s.IsEmpty())
    return;
  if (!info[1]->IsFunction()) {
    Nan::ThrowTypeError("SlipRock(): callback must be a function");
    return;
  }
  size_t len = s.ToLocalChecked()->Utf8Length();
  std::unique_ptr<char[]> buf(new char[len + 1]);
  s.ToLocalChecked()->WriteUtf8(buf.get(), len);
  buf[len] = '\0';
  if (strlen(buf.get()) != len) {
    Nan::ThrowTypeError("NUL not allowed in a filename");
    return;
  }
  Nan::AsyncQueueWorker(new SlipRockNewWorker(
      std::move(buf), len, new Nan::Callback(info[1].As<v8::Function>()),
      info.GetIsolate()));
}
}
NODE_MODULE(hello, SlipRock::Init)
