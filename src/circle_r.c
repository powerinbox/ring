#include <circle/circle.h>
#include <R.h>
#include <Rinternals.h>

static void circle_buffer_finalize(SEXP extPtr);
circle_buffer* circle_buffer_get(SEXP extPtr, int closed_error);

SEXP R_circle_buffer_create(SEXP r_size, SEXP r_stride) {
  size_t size = (size_t)INTEGER(r_size)[0], stride = INTEGER(r_stride)[0];

  circle_buffer *buffer = circle_buffer_create(size, stride);

  SEXP extPtr = PROTECT(R_MakeExternalPtr(buffer, R_NilValue, R_NilValue));
  R_RegisterCFinalizer(extPtr, circle_buffer_finalize);
  UNPROTECT(1);
  return extPtr;
}

SEXP R_circle_buffer_size(SEXP extPtr) {
  return ScalarInteger(circle_buffer_size(circle_buffer_get(extPtr, 1)));
}

SEXP R_circle_buffer_capacity(SEXP extPtr) {
  return ScalarInteger(circle_buffer_capacity(circle_buffer_get(extPtr, 1)));
}

SEXP R_circle_buffer_full(SEXP extPtr) {
  return ScalarLogical(circle_buffer_full(circle_buffer_get(extPtr, 1)));
}

SEXP R_circle_buffer_empty(SEXP extPtr) {
  return ScalarLogical(circle_buffer_empty(circle_buffer_get(extPtr, 1)));
}

SEXP R_circle_buffer_head(SEXP extPtr) {
  circle_buffer * buffer = circle_buffer_get(extPtr, 1);
  if (circle_buffer_empty(buffer)) {
    Rf_error("Buffer is empty");
  }
  SEXP ret = PROTECT(allocVector(RAWSXP, buffer->stride));
  memcpy(RAW(ret), circle_buffer_head(buffer), buffer->stride);
  UNPROTECT(1);
  return ret;
}

SEXP R_circle_buffer_tail(SEXP extPtr) {
  circle_buffer * buffer = circle_buffer_get(extPtr, 1);
  if (circle_buffer_empty(buffer)) {
    Rf_error("Buffer is empty");
  }
  SEXP ret = PROTECT(allocVector(RAWSXP, buffer->stride));
  memcpy(RAW(ret), circle_buffer_tail(buffer), buffer->stride);
  UNPROTECT(1);
  return ret;
}

SEXP R_circle_buffer_data(SEXP extPtr) {
  circle_buffer * buffer = circle_buffer_get(extPtr, 1);
  size_t len = buffer->stride * circle_buffer_capacity(buffer);
  SEXP ret = PROTECT(allocVector(RAWSXP, len));
  memcpy(RAW(ret), circle_buffer_data(buffer), len);
  UNPROTECT(1);
  return ret;
}

SEXP R_circle_buffer_head_pos(SEXP extPtr) {
  return ScalarInteger(circle_buffer_head_pos(circle_buffer_get(extPtr, 1)));
}

SEXP R_circle_buffer_tail_pos(SEXP extPtr) {
  return ScalarInteger(circle_buffer_tail_pos(circle_buffer_get(extPtr, 1)));
}

SEXP R_circle_buffer_bytes_free(SEXP extPtr) {
  return ScalarInteger(circle_buffer_bytes_free(circle_buffer_get(extPtr, 1)));
}

SEXP R_circle_buffer_bytes_used(SEXP extPtr) {
  return ScalarInteger(circle_buffer_bytes_used(circle_buffer_get(extPtr, 1)));
}

SEXP R_circle_buffer_reset(SEXP extPtr) {
  circle_buffer_reset(circle_buffer_get(extPtr, 1));
  return R_NilValue;
}

SEXP R_circle_buffer_memset(SEXP extPtr, SEXP c, SEXP len) {
  return ScalarInteger(circle_buffer_memset(circle_buffer_get(extPtr, 1),
                                            RAW(c)[0], INTEGER(len)[0]));
}

SEXP R_circle_buffer_memcpy_into(SEXP extPtr, SEXP src) {
  circle_buffer *buffer = circle_buffer_get(extPtr, 1);
  // TODO: warn about lack of stride fit.
  size_t count = LENGTH(src) / buffer->stride;
  circle_buffer_memcpy_into(buffer, RAW(src), count);
  return R_NilValue;
}

SEXP R_circle_buffer_memcpy_from(SEXP extPtr, SEXP r_count) {
  size_t count = INTEGER(r_count)[0];
  circle_buffer * buffer = circle_buffer_get(extPtr, 1);
  SEXP ret = PROTECT(allocVector(RAWSXP, count * buffer->stride));
  if (circle_buffer_memcpy_from(RAW(ret), buffer, count) == NULL) {
    Rf_error("Buffer underflow");
  }
  UNPROTECT(1);
  return ret;
}

void circle_buffer_finalize(SEXP extPtr) {
  circle_buffer *buffer = circle_buffer_get(extPtr, 0);
  if (buffer) {
    circle_buffer_free(buffer);
    R_ClearExternalPtr(extPtr);
  }
}

circle_buffer* circle_buffer_get(SEXP extPtr, int closed_error) {
  circle_buffer *buffer = NULL;
  if (TYPEOF(extPtr) != EXTPTRSXP) {
    Rf_error("Expected an external pointer");
  }
  buffer = (circle_buffer*) R_ExternalPtrAddr(extPtr);
  if (!buffer && closed_error) {
    Rf_error("circle_buffer already freed");
  }
  return buffer;
}

#include <R_ext/Rdynload.h>
static const R_CallMethodDef callMethods[] = {
  // circle_r
  {"Ccircle_buffer_create",      (DL_FUNC) &R_circle_buffer_create,      2},
  {"Ccircle_buffer_size",        (DL_FUNC) &R_circle_buffer_size,        1},
  {"Ccircle_buffer_capacity",    (DL_FUNC) &R_circle_buffer_capacity,    1},
  {"Ccircle_buffer_full",        (DL_FUNC) &R_circle_buffer_full,        1},
  {"Ccircle_buffer_empty",       (DL_FUNC) &R_circle_buffer_empty,       1},
  {"Ccircle_buffer_head",        (DL_FUNC) &R_circle_buffer_head,        1},
  {"Ccircle_buffer_tail",        (DL_FUNC) &R_circle_buffer_tail,        1},
  {"Ccircle_buffer_data",        (DL_FUNC) &R_circle_buffer_data,        1},
  {"Ccircle_buffer_head_pos",    (DL_FUNC) &R_circle_buffer_head_pos,    1},
  {"Ccircle_buffer_tail_pos",    (DL_FUNC) &R_circle_buffer_tail_pos,    1},
  {"Ccircle_buffer_bytes_free",  (DL_FUNC) &R_circle_buffer_bytes_free,  1},
  {"Ccircle_buffer_bytes_used",  (DL_FUNC) &R_circle_buffer_bytes_used,  1},
  {"Ccircle_buffer_reset",       (DL_FUNC) &R_circle_buffer_reset,       1},
  {"Ccircle_buffer_memset",      (DL_FUNC) &R_circle_buffer_memset,      3},
  {"Ccircle_buffer_memcpy_into", (DL_FUNC) &R_circle_buffer_memcpy_into, 2},
  {"Ccircle_buffer_memcpy_from", (DL_FUNC) &R_circle_buffer_memcpy_from, 2},
  {NULL,                         NULL,                                   0}
};

void R_init_circle(DllInfo *info) {
  R_registerRoutines(info, NULL, callMethods, NULL, NULL);
}
