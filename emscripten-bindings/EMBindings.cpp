// EMBindings.cpp
//
// MIT License
//
// Copyright(c) 2017 Rob Grindeland
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#include <emscripten/bind.h>
#include "FastNoise/FastNoise.h"

using namespace emscripten;

EMSCRIPTEN_BINDINGS(FastNoise) {
    // Module.NoiseType
    enum_<FastNoise::NoiseType>("NoiseType")
        .value("Value", FastNoise::Value)
        .value("ValueFractal", FastNoise::ValueFractal)
        .value("Perlin", FastNoise::Perlin)
        .value("PerlinFractal", FastNoise::PerlinFractal)
        .value("Simplex", FastNoise::Simplex)
        .value("SimplexFractal", FastNoise::SimplexFractal)
        .value("Cellular", FastNoise::Cellular)
        .value("WhiteNoise", FastNoise::WhiteNoise)
        .value("Cubic", FastNoise::Cubic)
        .value("CubicFractal", FastNoise::CubicFractal)
        ;
    // Module.Interp
    enum_<FastNoise::Interp>("Interp")
        .value("Linear", FastNoise::Linear)
        .value("Hermite", FastNoise::Hermite)
        .value("Quintic", FastNoise::Quintic)
        ;
    // Module.FractalType
    enum_<FastNoise::FractalType>("FractalType")
        .value("FBM", FastNoise::FBM)
        .value("Billow", FastNoise::Billow)
        .value("RigidMulti", FastNoise::RigidMulti)
        ;
    // Module.CellularDistanceFunction
    enum_<FastNoise::CellularDistanceFunction>("CellularDistanceFunction")
        .value("Euclidean", FastNoise::Euclidean)
        .value("Manhattan", FastNoise::Manhattan)
        .value("Natural", FastNoise::Natural)
        ;
    // Module.CellularReturnType
    enum_<FastNoise::CellularReturnType>("CellularReturnType")
        .value("CellValue", FastNoise::CellValue)
        .value("NoiseLookup", FastNoise::NoiseLookup)
        .value("Distance", FastNoise::Distance)
        .value("Distance2", FastNoise::Distance2)
        .value("Distance2Add", FastNoise::Distance2Add)
        .value("Distance2Sub", FastNoise::Distance2Sub)
        .value("Distance2Mul", FastNoise::Distance2Mul)
        .value("Distance2Div", FastNoise::Distance2Div)
        ;

    // Module.FastNoise
    class_<FastNoise>("FastNoise")
        .constructor<int>()

        // properties
        .property("seed", &FastNoise::GetSeed, &FastNoise::SetSeed)
        .property("frequency", &FastNoise::GetFrequency, &FastNoise::SetFrequency)
        .property("interp", &FastNoise::GetInterp, &FastNoise::SetInterp)
        .property("noiseType", &FastNoise::GetNoiseType, &FastNoise::SetNoiseType)
        .property("interp", &FastNoise::GetInterp, &FastNoise::SetInterp)
        .property("fractalOctaves", &FastNoise::GetFractalOctaves, &FastNoise::SetFractalOctaves)
        .property("fractalLacunarity", &FastNoise::GetFractalLacunarity, &FastNoise::SetFractalLacunarity)
        .property("fractalGain", &FastNoise::GetFractalGain, &FastNoise::SetFractalGain)
        .property("fractalType", &FastNoise::GetFractalType, &FastNoise::SetFractalType)
        .property("cellularDistanceFunction", &FastNoise::GetCellularDistanceFunction, &FastNoise::SetCellularDistanceFunction)
        // .property("cellularNoiseLookup", &FastNoise::GetCellularNoiseLookup, &FastNoise::SetCellularNoiseLookup)
        // .property("cellularDistance2Indices", &FastNoise::GetCellularDistance2Indices, &FastNoise::SetCellularDistance2Indices)
        .property("cellularJitter", &FastNoise::GetCellularJitter, &FastNoise::SetCellularJitter)
        .property("gradientPerturbAmp", &FastNoise::GetGradientPerturbAmp, &FastNoise::SetGradientPerturbAmp)

        // 2D
        .function("value2D", select_overload<FN_DECIMAL(FN_DECIMAL, FN_DECIMAL)const>(&FastNoise::GetValue))
        .function("valueFractal2D", select_overload<FN_DECIMAL(FN_DECIMAL, FN_DECIMAL)const>(&FastNoise::GetValueFractal))
        .function("perlin2D", select_overload<FN_DECIMAL(FN_DECIMAL, FN_DECIMAL)const>(&FastNoise::GetPerlin))
        .function("perlinFractal2D", select_overload<FN_DECIMAL(FN_DECIMAL, FN_DECIMAL)const>(&FastNoise::GetPerlinFractal))
        .function("simplex2D", select_overload<FN_DECIMAL(FN_DECIMAL, FN_DECIMAL)const>(&FastNoise::GetSimplex))
        .function("simplexFractal2D", select_overload<FN_DECIMAL(FN_DECIMAL, FN_DECIMAL)const>(&FastNoise::GetSimplexFractal))
        .function("cellular2D", select_overload<FN_DECIMAL(FN_DECIMAL, FN_DECIMAL)const>(&FastNoise::GetCellular))
        .function("whiteNoise2D", select_overload<FN_DECIMAL(FN_DECIMAL, FN_DECIMAL)const>(&FastNoise::GetWhiteNoise))
        .function("whiteNoiseInt2D", select_overload<FN_DECIMAL(int, int)const>(&FastNoise::GetWhiteNoiseInt))
        .function("cubic2D", select_overload<FN_DECIMAL(FN_DECIMAL, FN_DECIMAL)const>(&FastNoise::GetCubic))
        .function("cubicFractal2D", select_overload<FN_DECIMAL(FN_DECIMAL, FN_DECIMAL)const>(&FastNoise::GetCubicFractal))
        .function("getNoise2D", select_overload<FN_DECIMAL(FN_DECIMAL, FN_DECIMAL)const>(&FastNoise::GetNoise))
        // .function("gradientPerturb2D", select_overload<void(FN_DECIMAL&, FN_DECIMAL&)const>(&FastNoise::GradientPerturb))
        // .function("gradientPerturbFractal2D", select_overload<void(FN_DECIMAL&, FN_DECIMAL&)const>(&FastNoise::GradientPerturbFractal))

        // 3D
        .function("value3D", select_overload<FN_DECIMAL(FN_DECIMAL, FN_DECIMAL, FN_DECIMAL)const>(&FastNoise::GetValue))
        .function("valueFractal3D", select_overload<FN_DECIMAL(FN_DECIMAL, FN_DECIMAL, FN_DECIMAL)const>(&FastNoise::GetValueFractal))
        .function("perlin3D", select_overload<FN_DECIMAL(FN_DECIMAL, FN_DECIMAL, FN_DECIMAL)const>(&FastNoise::GetPerlin))
        .function("perlinFractal3D", select_overload<FN_DECIMAL(FN_DECIMAL, FN_DECIMAL, FN_DECIMAL)const>(&FastNoise::GetPerlinFractal))
        .function("simplex3D", select_overload<FN_DECIMAL(FN_DECIMAL, FN_DECIMAL, FN_DECIMAL)const>(&FastNoise::GetSimplex))
        .function("simplexFractal3D", select_overload<FN_DECIMAL(FN_DECIMAL, FN_DECIMAL, FN_DECIMAL)const>(&FastNoise::GetSimplexFractal))
        .function("cellular3D", select_overload<FN_DECIMAL(FN_DECIMAL, FN_DECIMAL, FN_DECIMAL)const>(&FastNoise::GetCellular))
        .function("whiteNoise3D", select_overload<FN_DECIMAL(FN_DECIMAL, FN_DECIMAL, FN_DECIMAL)const>(&FastNoise::GetWhiteNoise))
        .function("whiteNoiseInt3D", select_overload<FN_DECIMAL(int, int, int)const>(&FastNoise::GetWhiteNoiseInt))
        .function("cubic3D", select_overload<FN_DECIMAL(FN_DECIMAL, FN_DECIMAL, FN_DECIMAL)const>(&FastNoise::GetCubic))
        .function("cubicFractal3D", select_overload<FN_DECIMAL(FN_DECIMAL, FN_DECIMAL, FN_DECIMAL)const>(&FastNoise::GetCubicFractal))
        .function("getNoise3D", select_overload<FN_DECIMAL(FN_DECIMAL, FN_DECIMAL, FN_DECIMAL)const>(&FastNoise::GetNoise))
        // .function("gradientPerturb3D", select_overload<void(FN_DECIMAL&, FN_DECIMAL&, FN_DECIMAL&)const>(&FastNoise::GradientPerturb))
        // .function("gradientPerturbFractal3D", select_overload<void(FN_DECIMAL&, FN_DECIMAL&, FN_DECIMAL&)const>(&FastNoise::GradientPerturbFractal))

        // 4D
        .function("simplex4D", select_overload<FN_DECIMAL(FN_DECIMAL, FN_DECIMAL, FN_DECIMAL, FN_DECIMAL)const>(&FastNoise::GetSimplex))
        .function("whiteNoise4D", select_overload<FN_DECIMAL(FN_DECIMAL, FN_DECIMAL, FN_DECIMAL, FN_DECIMAL)const>(&FastNoise::GetWhiteNoise))
        .function("whiteNoiseInt4D", select_overload<FN_DECIMAL(int, int, int, int)const>(&FastNoise::GetWhiteNoiseInt))
        ;
}
