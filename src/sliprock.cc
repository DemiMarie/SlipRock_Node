#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <memory>
#include <nan.h>
#include <sliprock.h>

namespace {

struct SlipRockBaseWorker : public Nan::AsyncWorker {
  SlipRockBaseWorker(Nan::Callback *callback, v8::Isolate *isolate,
                     SliprockConnection *con)
      : Nan::AsyncWorker(callback), err(0), isolate(isolate), con(con) {}
  int err = 0;
  v8::Isolate *isolate;
  SliprockConnection *con;
  void HandleErrorCallback() override final {
    auto err = Nan::ErrnoException(this->err, "sliprock_socket",
                                   "Failed to create socket", "");
    v8::Local<v8::Value> args[1] = {err};
    callback->Call(1, args);
  }
};

struct SlipRockCloseWorker : public SlipRockBaseWorker {
  SlipRockCloseWorker(Nan::Callback *callback, v8::Isolate *isolate,
                      SliprockConnection *con)
      : SlipRockBaseWorker(callback, isolate, con) {}

  void Execute() override {
    errno = 0;
    sliprock_close(this->con);
    this->err = errno;
    if (this->err) {
      this->SetErrorMessage("Failed to close socket");
    }
  }

  void HandleOKCallback() override {
    v8::Local<v8::Value> args[1] = {Nan::Null()};
    callback->Call(1, args);
  }

private:
  int err;
  SliprockConnection *con;
};

static Nan::Persistent<v8::Function> &constructor() {
  static Nan::Persistent<v8::Function> mycon;
  return mycon;
}

struct SlipRock : public Nan::ObjectWrap {
  static NAN_METHOD(NewInstance) { info.GetReturnValue().Set(info.This()); }
  static NAN_METHOD(New);
  static NAN_METHOD(Close) {
    SlipRock *s = reinterpret_cast<SlipRock *>(
        Nan::GetInternalFieldPointer(info.Holder(), 0));
    assert(s);
    sliprock_close(s->con);
    s->con = nullptr;
  }
  struct SliprockConnection *con;

  SlipRock(SliprockConnection *con) : con(con) {}

  ~SlipRock() { sliprock_close(this->con); }

  static NAN_MODULE_INIT(Init) {
    auto tpl = Nan::New<v8::FunctionTemplate>(SlipRock::NewInstance);
    tpl->SetClassName(Nan::New("SlipRock").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    Nan::SetPrototypeMethod(tpl, "close", SlipRock::Close);
    constructor().Reset(Nan::GetFunction(tpl).ToLocalChecked());
    auto q = Nan::New<v8::FunctionTemplate>(SlipRock::New);
    Nan::Set(target, Nan::New("new").ToLocalChecked(),
             Nan::GetFunction(q).ToLocalChecked());
  }
  void Wrap(v8::Local<v8::Object> o) { return this->Nan::ObjectWrap::Wrap(o); }
};

struct SlipRockAcceptWorker : public SlipRockBaseWorker {
  SlipRockAcceptWorker(Nan::Callback *callback, v8::Isolate *isolate,
                       SliprockConnection *con)
      : SlipRockBaseWorker(callback, isolate, con) {}

  void Execute() override {
    errno = 0;
    this->res = sliprock_accept(this->con);
    this->err = errno;
    if (this->err) {
      this->SetErrorMessage("Failed to accept from socket");
    }
  }

  void HandleOKCallback() override {
    v8::Local<v8::Value> args[2] = {Nan::Null(),
                                    Nan::New<v8::Int32>(this->res)};
    callback->Call(2, args);
  }

private:
  int err;
  int res;
  SliprockConnection *con;
};

// Worker that queues the object
struct SlipRockNewWorker : public SlipRockBaseWorker {
  SlipRockNewWorker(std::unique_ptr<char[]> buf, size_t length,
                    Nan::Callback *callback, v8::Isolate *isolate)
      : SlipRockBaseWorker(callback, isolate, nullptr), buf(std::move(buf)),
        size(length) {}
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
    (new SlipRock(this->con))->Wrap(instance);
    con = nullptr;
    v8::Local<v8::Value> args[2] = {Nan::Null(), instance};
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
  Nan::AsyncQueueWorker(new SlipRockNewWorker(
      std::move(buf), len, new Nan::Callback(info[1].As<v8::Function>()),
      info.GetIsolate()));
}
}
NODE_MODULE(hello, SlipRock::Init)
