(ql:quickload '(:cffi :uiop))
(cffi:load-foreign-library "/home/aswin/workspace/quickjs-2021-03-27/libnodeclone.so")
(cffi:defcfun ("qjs_init" qjs-init) :void)
(cffi:defcfun ("qjs_eval" qjs-eval) :string (code :string))
(cffi:defcfun ("qjs_cleanup" qjs-cleanup) :void)
(cffi:defcfun ("qjs_register_lisp_callback" qjs-set-callback) :void (cb :pointer))
(cffi:defcfun ("invoke_callback" invoke-callback) :void (id :int))

;; libuv
(cffi:defcfun ("uv_host_init" uv-init) :void)
(cffi:defcfun ("uv_host_run_once" uv-run-once) :void)
(cffi:defcfun ("uv_host_set_timeout" uv-set-timeout) :void (id :int) (delay :int))
(cffi:defcfun ("uv_register_lisp_callback" uv-set-callback) :void (cb :pointer))
(defvar *callback-queue* '())

(defun enqueue-callback (id)
  (format t "[Lisp] Enqueueing callback ID: ~A~%" id)
  (push id *callback-queue*))

(defun drain-callbacks ()
  (dolist (id (reverse *callback-queue*))
    (format t "[Lisp] Executing callback ID: ~A~%" id)
    (invoke-callback id))
  (setf *callback-queue* nil))

(cffi:defcallback enqueue-callback-c :void ((id :int))
  (enqueue-callback id))

(defun eval-js (code)
  (qjs-eval code))

(defun nodeclone-init ()
  (qjs-init)
  (uv-init)
  (qjs-set-callback (cffi:callback enqueue-callback-c))
  (uv-set-callback (cffi:callback enqueue-callback-c))
  (format t "[Nodeclone] Initialized.~%"))

(defun run-event-loop (&key (max-seconds 5))
  (let ((start (get-internal-real-time)))
    (loop until (> (- (get-internal-real-time) start) (* max-seconds internal-time-units-per-second))
          do (progn (uv-run-once) (drain-callbacks) (sleep 0.01)))))

(defun run-js-file (filename)
  "Load and run a JavaScript file via QuickJS."
  (let ((code (uiop:read-file-string filename)))
    (eval-js code)
    (run-event-loop)))


(defun main ()
  (nodeclone-init)
  (let ((args (uiop:command-line-arguments)))
    (if args
        (run-js-file (first args))
        (format t "Usage: sbcl --load nodeclone.lisp --eval \"(main)\" <script.js>~%")))
  (quit))
