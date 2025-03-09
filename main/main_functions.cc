/* Copyright 2020 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/


#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include "main_functions.h"
#include "model.h"
#include "output_handler.h"
#include "mqtt.h"

// Globals, used for compatibility with Arduino-style sketches.
namespace {
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;

constexpr int kTensorArenaSize = 60*1024; //For contiguous memory allocation used by the framework
uint8_t tensor_arena[kTensorArenaSize];
}  // namespace
// The name of this function is important for Arduino compatibility.
void setup() {
  MicroPrintf("Setting up TFLite");
  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  model = tflite::GetModel(g_model);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    MicroPrintf("Model provided is schema version %d not equal to supported "
                "version %d.", model->version(), TFLITE_SCHEMA_VERSION);
    return;
  }
  // Pull in only the operation implementations we need.
  //A way for optimizing the memory
  static tflite::MicroMutableOpResolver<4> resolver;

// Agrega operaciones disponibles
if (resolver.AddReshape() != kTfLiteOk) {
    MicroPrintf("Reshape not created!");
    return;
}

if (resolver.AddFullyConnected() != kTfLiteOk) {
    MicroPrintf("Fully Connected not created!");
    return;
}

if (resolver.AddSoftmax() != kTfLiteOk) {
    MicroPrintf("Softmax not created!");
    return;
}

  MicroPrintf("Good after Resolver!");
  // Build an interpreter to run the model with.
  static tflite::MicroInterpreter static_interpreter(
      model, resolver, tensor_arena, kTensorArenaSize);
  interpreter = &static_interpreter;
  MicroPrintf("Good after Interpreter!");
  // Allocate memory from the tensor_arena for the model's tensors.
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  MicroPrintf("Good after Allocation!");
  if (allocate_status != kTfLiteOk) {
    MicroPrintf("AllocateTensors() failed");
    return;
  }
  
  // Obtain pointers to the model's input and output tensors.
  input = interpreter->input(0);


  MicroPrintf("All OK!");
}


void extract_features(int16_t *array, float *extractionArray, int length) {
    float prom = 0, dv = 0;

    for (int i = 0; i < length; i++) {
        prom += array[i];
    }
    float mtd = prom / length;
    extractionArray[0] = mtd;

    for (int i = 0; i < length; i++) {
        dv += (array[i] - mtd) * (array[i] - mtd);
    }
    dv /= length;
    extractionArray[1] = sqrtf(dv); // Usa sqrtf para float
}


// The name of this function is important for Arduino compatibility.
int loop(int16_t *Ax,int16_t *Ay,int16_t *Az) {
  
  // Place the quantized input in the model's input tensor
  // Be carfeful with data type. Expected input is signed but camera frame is uint8
    int predic=0;
    int slidingWindow = 6;
    float features[5];
    float data[30];
    //Se limpia el vector de datos
    for(int i=0; i<=30; i++){
        data[i] = 0;
    }
    //Se verifica que tengan datos
    if (Ax == nullptr || Ay == nullptr || Az == nullptr) {
    MicroPrintf("Error: Null pointer detected in sensor data.");
    
    }
    //se obtiene la extración de características, el promedio y la desviación estandar
		for (int i = 0; i < slidingWindow-1; i++) {
		        // Extraer características de X, Y y Z para cada ventana
		        extract_features(Ax + i * slidingWindow, features, slidingWindow);
		        data[i] = features[0];            // X_mean
		        data[15 + i] = features[1];       // X_std

		        extract_features(Ay + i * slidingWindow, features, slidingWindow);
		        data[5 + i] = features[0];        // Y_mean
		        data[20 + i] = features[1];       // Y_std

		        extract_features(Az + i * slidingWindow, features, slidingWindow);
		        data[10 + i] = features[0];       // Z_mean
		        data[25 + i] = features[1];       // Z_std
        }
//Se mandan los datos al modelo a través del puntero input
for (int i = 0; i < 30; i++) {
    input->data.f[i] = data[i];
}
  
  // Run inference, and report any error
  TfLiteStatus invoke_status = interpreter->Invoke();
  if (invoke_status != kTfLiteOk) {
    MicroPrintf("Invoke failed");
   // return;
  }
int max_index =0; 
output = interpreter->output(0);
float category_scores[7];
//es obtienen los puntajes del puntero de salida del modelo
for (int i = 0; i < 6; i++) {
    category_scores[i] = output->data.f[i];
    MicroPrintf("Score: %.10e ",category_scores[i]);
} 

float max_score = category_scores[0];
//se acomodan para ver cual fue el mayor puntaje
for (int i = 1; i < 6; i++) {
    if (category_scores[i] > max_score) {
        max_index = i;
        max_score = category_scores[i];
    }
}
//se manda el mayor puntaje 
  predic =  max_index;

return predic;

}
