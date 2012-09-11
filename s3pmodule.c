/**
   libS3P - Python bindings for the Super Simple Streaming Protocol library.

   Copyright (C) 2012 Nicholas Pascucci (npascut1@gmail.com)

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <Python.h>
#include <stdint.h>
#include <stddef.h>
#include <s3p.h>

static PyObject * py_s3p_build(PyObject *self, PyObject *args);
static PyObject * py_s3p_read(PyObject *self, PyObject *args);

static PyMethodDef S3PMethods[] = {
  {"build", py_s3p_build, METH_VARARGS, 

   "s3p.build(str): Build an S3P packet from a string.\n\n"
   "This function wraps the C function s3p_build, which constructs Super\n"
   "Simple Streaming Protocol packets. S3P packets can encapsulate any binary\n"
   "data structured as a byte sequence.\n\n"
   "Arguments:\n"
   "str -- a byte sequence to be encapsulated.\n\n"
   "Returns:\n"
   "A new Python string containing the encapsulated data.\n\n"
},

  {"read", py_s3p_read, METH_VARARGS, 
   "s3p.read(str): Read an S3P packet from a string.\n\n"
   "This function wraps the C function s3p_read, which extracts data from\n"
   "Super Simple Streaming Protocol Packets. S3P packets contain a checksum\n"
   "which allows for extremely basic error detection (but not correction).\n"
   "s3p.read() will raise a ValueError if the checksum does not match the\n"
   "data in the packet.\n\n"
   "Arguments:\n"
   "str -- an S3P packet to read.\n\n"
   "Returns:\n"
   "A new Python string containing the packet's data.\n\n"
   "Raises:\n"
   "ValueError if the packet's checksum doesn't match the data.\n"
   "ValueError if the packet's formatting doesn't match the S3P protocol.\n\n"},

  {NULL, NULL, 0, NULL} // Sentinel
};

PyMODINIT_FUNC inits3p(void){
  Py_InitModule("s3p", S3PMethods);
}

/**
   Wrapper function for s3p_build(). 

   In Python, the signature for this function is:
   
   packet = s3p.build(data)

   where packet and data are strings, similar to the standard library's struct
   module.
 */
static PyObject * py_s3p_build(PyObject *self, PyObject *args){
  const uint8_t *data;
  int data_size = 0;

  if(!PyArg_ParseTuple(args, "s#", &data, &data_size)){
    return NULL;
  }

  size_t psize;
  size_t out_size = 2*data_size + S3P_OVERHEAD;
  uint8_t *out = malloc(out_size);
  
  S3P_ERR err = s3p_build(data, (size_t) data_size, out, out_size, &psize);

  if(S3P_SUCCESS == err){
    PyObject *py_string = Py_BuildValue("s#", out, psize);
    free(out);
    return py_string;
  } else {
    free(out);
    char err_message[100] = { 0x00 };
    sprintf(err_message, 
            "s3p_read returned an unexpected error: %d.\n"
            "Please file a bug report so we can fix this!", err);
    PyErr_SetString(PyExc_EnvironmentError, err_message);
  }
  return NULL;
}

/**
   Wrapper function for s3p_read(). 

   In Python, the signature for this function is:
   
   data = s3p.read(packet)

   where data and packet are strings, similar to the standard library's struct
   module.
 */
static PyObject * py_s3p_read(PyObject *self, PyObject *args){
  const uint8_t *packet;
  int packet_size = 0;

  if(!PyArg_ParseTuple(args, "s#", &packet, &packet_size)){
    return NULL;
  }

  size_t psize;
  size_t out_size = packet_size;
  uint8_t *out = malloc(out_size);
  
  S3P_ERR err = s3p_read(packet, (size_t) packet_size, out, out_size, &psize);

  if(S3P_SUCCESS == err){
    PyObject *py_string = Py_BuildValue("s#", out, psize);
    free(out);
    return py_string;
  } else {
    free(out);
    char err_message[150] = { 0x00 };

    switch(err){
      // TODO Add custom exceptions that are more descriptive.
    case S3P_CHECKSUM_ERR:
      PyErr_SetString(PyExc_ValueError, 
                      "The received and calculated checksums don't match.");
      break;
    case S3P_PARSE_FAILURE:
      PyErr_SetString(PyExc_ValueError, 
                      "The packet failed to parse.");
      break;
    default:
      sprintf(err_message, 
              "s3p_read returned an unexpected error: %d.\n"
              "Please file a bug report so we can fix this!", err);
      PyErr_SetString(PyExc_EnvironmentError, err_message);
      break;
    }
  }
  return NULL;
}
