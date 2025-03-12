/*
 * Copyright (c) 2025, BlackBerry Limited. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// adapted from the FastLED library (https://fastled.io/) 

#include <stdint.h>
#include <math.h>
#include "mini_fastled.h"


// pixel set functions
ws2811_led_t *get(struct rgb_pixel_set *rps, unsigned index)
{
    return &(rps->leds[index]);
}

void copyFrom(struct rgb_pixel_set *rps_to, int to_start, int to_end,
              struct rgb_pixel_set *rps_from, int from_start, int from_end)
{
    if (fabs(to_start - to_end) != fabs(from_start - from_end))
    {
        // invalid copy ... skip for now ...
        return;
    }

    int to_inc = 1;
    if (to_start > to_end)
    {
        to_inc = -1;
    }
    int from_inc = 1;
    if (from_start > from_end)
    {
        from_inc = -1;
    }

    for (int index = 0; index < fabs(to_start - to_end); index++)
    {
        rps_to->leds[index * to_inc + to_start] = rps_from->leds[index * from_inc + from_start];
    }
}

void nscale8(struct rgb_pixel_set *rps, fract8 scale)
{
    for (uint16_t index = 0; index < rps->length; index++)
    {
        uint8_t red = (rps->leds[index] >> LED_SHIFT_R) % 0xff;
        uint8_t green = (rps->leds[index] >> LED_SHIFT_G) % 0xff;
        uint8_t blue = (rps->leds[index] >> LED_SHIFT_B) % 0xff;

        nscale8x3(&red, &green, &blue, scale);

        rps->leds[index] = (red << LED_SHIFT_R) + (green << LED_SHIFT_G) + (blue << LED_SHIFT_B);
    }
}

void fadeToBlackBy(struct rgb_pixel_set *rps, uint8_t fadefactor)
{
    rps->nscale8(rps, 255 - fadefactor);
}

/*
void fill_rainbow(struct CRGB *targetArray, int numToFill,
                  uint8_t initialhue,
                  uint8_t deltahue)
{
    CHSV hsv;
    hsv.hue = initialhue;
    hsv.val = 255;
    hsv.sat = 240;
    for (int i = 0; i < numToFill; ++i)
    {
        targetArray[i] = hsv;
        hsv.hue += deltahue;
    }
}
*/

void fill_rainbow(struct rgb_pixel_set *rps, uint8_t initialhue, uint8_t deltahue)
{
    if (rps->direction >= 0)
    {
        uint8_t hue = initialhue;
        for (uint16_t index = 0; index < rps->length; index++)
        {
            rps->leds[index] = CHSV(hue, 255, 255);
            hue += deltahue;
        }
        /*
                FUNCTION_FILL_RAINBOW(leds,len,initialhue,deltahue);
                FUNCTION_FILL_RAINBOW(leds + len + 1, -len, initialhue - deltahue * (len+1), -deltahue);
            return *this;
        */
    }
    else
    {
    }
}

/*
    /// Get the size of this set
    /// @return the size of the set, in number of LEDs
    int size() { return abs(len); }

    /// Whether or not this set goes backwards
    /// @return whether or not the set is backwards
    bool reversed() { return len < 0; }

    /// Do these sets point to the same thing? Note that this is different from the contents of the set being the same.
    bool operator==(const CPixelView & rhs) const { return leds == rhs.leds && len == rhs.len && dir == rhs.dir; }

    /// Do these sets point to different things? Note that this is different from the contents of the set being the same.
    bool operator!=(const CPixelView & rhs) const { return leds != rhs.leds || len != rhs.len || dir != rhs.dir; }

    /// Access a single element in this set, just like an array operator
    inline PIXEL_TYPE & operator[](int x) const { if(dir & 0x80) { return leds[-x]; } else { return leds[x]; } }

    /// Access an inclusive subset of the LEDs in this set.
    /// @note The start point can be greater than end, which will
    /// result in a reverse ordering for many functions (useful for mirroring).
    /// @param start the first element from this set for the new subset
    /// @param end the last element for the new subset
    inline CPixelView operator()(int start, int end) { if(dir & 0x80) { return CPixelView(leds+len+1, -len-start-1, -len-end-1); } else { return CPixelView(leds, start, end); } }

    // Access an inclusive subset of the LEDs in this set, starting from the first.
    // @param end the last element for the new subset
    // @todo Not sure i want this? inline CPixelView operator()(int end) { return CPixelView(leds, 0, end); }

    /// Return the reverse ordering of this set
    inline CPixelView operator-() { return CPixelView(leds, len - dir, 0); }

    /// Return a pointer to the first element in this set
    inline operator PIXEL_TYPE* () const { return leds; }

    /// Assign the passed in color to all elements in this set
    /// @param color the new color for the elements in the set
    inline CPixelView & operator=(const PIXEL_TYPE & color) {
        for(iterator pixel = begin(), _end = end(); pixel != _end; ++pixel) { (*pixel) = color; }
        return *this;
    }

    /// Print debug data to serial, disabled for release.
    /// Edit this file to re-enable these for debugging purposes.
    void dump() const {
        /// @code
        /// Serial.print("len: "); Serial.print(len); Serial.print(", dir:"); Serial.print((int)dir);
        /// Serial.print(", range:"); Serial.print((uint32_t)leds); Serial.print("-"); Serial.print((uint32_t)end_pos);
        /// Serial.print(", diff:"); Serial.print((int32_t)(end_pos - leds));
        /// Serial.println("");
        /// @endcode
    }

    /// Copy the contents of the passed-in set to our set.
    /// @note If one set is smaller than the other, only the
    /// smallest number of items will be copied over.
    inline CPixelView & operator=(const CPixelView & rhs) {
        for(iterator pixel = begin(), rhspixel = rhs.begin(), _end = end(), rhs_end = rhs.end(); (pixel != _end) && (rhspixel != rhs_end); ++pixel, ++rhspixel) {
            (*pixel) = (*rhspixel);
        }
        return *this;
    }

    /// @name Modification/Scaling Operators
    /// @{

    /// Add the passed in value to all channels for all of the pixels in this set
    inline CPixelView & addToRGB(uint8_t inc) { for(iterator pixel = begin(), _end = end(); pixel != _end; ++pixel) { (*pixel) += inc; } return *this; }
    /// Add every pixel in the other set to this set
    inline CPixelView & operator+=(CPixelView & rhs) { for(iterator pixel = begin(), rhspixel = rhs.begin(), _end = end(), rhs_end = rhs.end(); (pixel != _end) && (rhspixel != rhs_end); ++pixel, ++rhspixel) { (*pixel) += (*rhspixel); } return *this; }

    /// Subtract the passed in value from all channels for all of the pixels in this set
    inline CPixelView & subFromRGB(uint8_t inc) { for(iterator pixel = begin(), _end = end(); pixel != _end; ++pixel) { (*pixel) -= inc; } return *this; }
    /// Subtract every pixel in the other set from this set
    inline CPixelView & operator-=(CPixelView & rhs) { for(iterator pixel = begin(), rhspixel = rhs.begin(), _end = end(), rhs_end = rhs.end(); (pixel != _end) && (rhspixel != rhs_end); ++pixel, ++rhspixel) { (*pixel) -= (*rhspixel); } return *this; }

    /// Increment every pixel value in this set
    inline CPixelView & operator++() { for(iterator pixel = begin(), _end = end(); pixel != _end; ++pixel) { (*pixel)++; } return *this; }
    /// Increment every pixel value in this set
    inline CPixelView & operator++(int DUMMY_ARG) {
        FASTLED_UNUSED(DUMMY_ARG);
        for(iterator pixel = begin(), _end = end(); pixel != _end; ++pixel) {
            (*pixel)++;
        }
        return *this;
    }

    /// Decrement every pixel value in this set
    inline CPixelView & operator--() { for(iterator pixel = begin(), _end = end(); pixel != _end; ++pixel) { (*pixel)--; } return *this; }
    /// Decrement every pixel value in this set
    inline CPixelView & operator--(int DUMMY_ARG) {
        FASTLED_UNUSED(DUMMY_ARG);
        for(iterator pixel = begin(), _end = end(); pixel != _end; ++pixel) {
            (*pixel)--;
        }
        return *this;
    }

    /// Divide every LED by the given value
    inline CPixelView & operator/=(uint8_t d) { for(iterator pixel = begin(), _end = end(); pixel != _end; ++pixel) { (*pixel) /= d; } return *this; }
    /// Shift every LED in this set right by the given number of bits
    inline CPixelView & operator>>=(uint8_t d) { for(iterator pixel = begin(), _end = end(); pixel != _end; ++pixel) { (*pixel) >>= d; } return *this; }
    /// Multiply every LED in this set by the given value
    inline CPixelView & operator*=(uint8_t d) { for(iterator pixel = begin(), _end = end(); pixel != _end; ++pixel) { (*pixel) *= d; } return *this; }

    /// Scale every LED by the given scale
    inline CPixelView & nscale8_video(uint8_t scaledown) { for(iterator pixel = begin(), _end = end(); pixel != _end; ++pixel) { (*pixel).nscale8_video(scaledown); } return *this;}
    /// Scale down every LED by the given scale
    inline CPixelView & operator%=(uint8_t scaledown) { for(iterator pixel = begin(), _end = end(); pixel != _end; ++pixel) { (*pixel).nscale8_video(scaledown); } return *this; }
    /// Fade every LED down by the given scale
    inline CPixelView & fadeLightBy(uint8_t fadefactor) { return nscale8_video(255 - fadefactor); }

    /// Scale every LED by the given scale
    inline CPixelView & nscale8(uint8_t scaledown) { for(iterator pixel = begin(), _end = end(); pixel != _end; ++pixel) { (*pixel).nscale8(scaledown); } return *this; }
    /// Scale every LED by the given scale
    inline CPixelView & nscale8(PIXEL_TYPE & scaledown) { for(iterator pixel = begin(), _end = end(); pixel != _end; ++pixel) { (*pixel).nscale8(scaledown); } return *this; }
    /// Scale every LED in this set by every led in the other set
    inline CPixelView & nscale8(CPixelView & rhs) { for(iterator pixel = begin(), rhspixel = rhs.begin(), _end = end(), rhs_end = rhs.end(); (pixel != _end) && (rhspixel != rhs_end); ++pixel, ++rhspixel) { (*pixel).nscale8((*rhspixel)); } return *this; }

    /// Fade every LED down by the given scale
    inline CPixelView & fadeToBlackBy(uint8_t fade) { return nscale8(255 - fade); }

    /// Apply the PIXEL_TYPE |= operator to every pixel in this set with the given PIXEL_TYPE value.
    /// With CRGB, this brings up each channel to the higher of the two values
    /// @see CRGB::operator|=
    inline CPixelView & operator|=(const PIXEL_TYPE & rhs) { for(iterator pixel = begin(), _end = end(); pixel != _end; ++pixel) { (*pixel) |= rhs; } return *this; }
    /// Apply the PIXEL_TYPE |= operator to every pixel in this set with every pixel in the passed in set.
    /// @copydetails operator|=(const PIXEL_TYPE&)
    inline CPixelView & operator|=(const CPixelView & rhs) { for(iterator pixel = begin(), rhspixel = rhs.begin(), _end = end(), rhs_end = rhs.end(); (pixel != _end) && (rhspixel != rhs_end); ++pixel, ++rhspixel) { (*pixel) |= (*rhspixel); } return *this; }
    /// Apply the PIXEL_TYPE |= operator to every pixel in this set.
    /// @copydetails operator|=(const PIXEL_TYPE&)
    inline CPixelView & operator|=(uint8_t d) { for(iterator pixel = begin(), _end = end(); pixel != _end; ++pixel) { (*pixel) |= d; } return *this; }

    /// Apply the PIXEL_TYPE &= operator to every pixel in this set with the given PIXEL_TYPE value.
    /// With CRGB, this brings up each channel down to the lower of the two values
    /// @see CRGB::operator&=
    inline CPixelView & operator&=(const PIXEL_TYPE & rhs) { for(iterator pixel = begin(), _end = end(); pixel != _end; ++pixel) { (*pixel) &= rhs; } return *this; }
    /// Apply the PIXEL_TYPE &= operator to every pixel in this set with every pixel in the passed in set.
    /// @copydetails operator&=(const PIXEL_TYPE&)
    inline CPixelView & operator&=(const CPixelView & rhs) { for(iterator pixel = begin(), rhspixel = rhs.begin(), _end = end(), rhs_end = rhs.end(); (pixel != _end) && (rhspixel != rhs_end); ++pixel, ++rhspixel) { (*pixel) &= (*rhspixel); } return *this; }
    /// Apply the PIXEL_TYPE &= operator to every pixel in this set with the passed in value.
    /// @copydetails operator&=(const PIXEL_TYPE&)
    inline CPixelView & operator&=(uint8_t d) { for(iterator pixel = begin(), _end = end(); pixel != _end; ++pixel) { (*pixel) &= d; } return *this; }

    /// @} Modification/Scaling Operators


    /// Returns whether or not any LEDs in this set are non-zero
    inline operator bool() { for(iterator pixel = begin(), _end = end(); pixel != _end; ++pixel) { if((*pixel)) return true; } return false; }


    /// @name Color Util Functions
    /// @{

    /// Fill all of the LEDs with a solid color
    /// @param color the color to fill with
    inline CPixelView & fill_solid(const PIXEL_TYPE & color) { *this = color; return *this; }
    /// @copydoc CPixelView::fill_solid(const PIXEL_TYPE&)
    inline CPixelView & fill_solid(const CHSV & color) { *this = color; return *this; }

    /// Fill all of the LEDs with a smooth HSV gradient between two HSV colors.
    /// @param startcolor the starting color in the gradient
    /// @param endcolor the end color for the gradient
    /// @param directionCode the direction to travel around the color wheel
    /// @see ::fill_gradient(T*, uint16_t, const CHSV&, const CHSV&, TGradientDirectionCode)
    inline CPixelView & fill_gradient(const CHSV & startcolor, const CHSV & endcolor, TGradientDirectionCode directionCode  = SHORTEST_HUES) {
        if(dir >= 0) {
            FUNCTION_FILL_GRADIENT(leds,len,startcolor, endcolor, directionCode);
        } else {
            FUNCTION_FILL_GRADIENT(leds + len + 1, (-len), endcolor, startcolor, directionCode);
        }
        return *this;
    }

    /// Fill all of the LEDs with a smooth HSV gradient between three HSV colors.
    /// @param c1 the starting color in the gradient
    /// @param c2 the middle color for the gradient
    /// @param c3 the end color for the gradient
    /// @param directionCode the direction to travel around the color wheel
    /// @see ::fill_gradient(T*, uint16_t, const CHSV&, const CHSV&, const CHSV&, TGradientDirectionCode)
    inline CPixelView & fill_gradient(const CHSV & c1, const CHSV & c2, const CHSV &  c3, TGradientDirectionCode directionCode = SHORTEST_HUES) {
        if(dir >= 0) {
            FUNCTION_FILL_GRADIENT3(leds, len, c1, c2, c3, directionCode);
        } else {
            FUNCTION_FILL_GRADIENT3(leds + len + 1, -len, c3, c2, c1, directionCode);
        }
        return *this;
    }

    /// Fill all of the LEDs with a smooth HSV gradient between four HSV colors.
    /// @param c1 the starting color in the gradient
    /// @param c2 the first middle color for the gradient
    /// @param c3 the second middle color for the gradient
    /// @param c4 the end color for the gradient
    /// @param directionCode the direction to travel around the color wheel
    /// @see ::fill_gradient(T*, uint16_t, const CHSV&, const CHSV&, const CHSV&, const CHSV&, TGradientDirectionCode)
    inline CPixelView & fill_gradient(const CHSV & c1, const CHSV & c2, const CHSV & c3, const CHSV & c4, TGradientDirectionCode directionCode = SHORTEST_HUES) {
        if(dir >= 0) {
            FUNCTION_FILL_GRADIENT4(leds, len, c1, c2, c3, c4, directionCode);
        } else {
            FUNCTION_FILL_GRADIENT4(leds + len + 1, -len, c4, c3, c2, c1, directionCode);
        }
        return *this;
    }

    /// Fill all of the LEDs with a smooth RGB gradient between two RGB colors.
    /// @param startcolor the starting color in the gradient
    /// @param endcolor the end color for the gradient
    /// @param directionCode the direction to travel around the color wheel
    /// @see ::fill_gradient_RGB(CRGB*, uint16_t, const CRGB&, const CRGB&)
    inline CPixelView & fill_gradient_RGB(const PIXEL_TYPE & startcolor, const PIXEL_TYPE & endcolor, TGradientDirectionCode directionCode  = SHORTEST_HUES) {
        FASTLED_UNUSED(directionCode); // TODO: why is this not used?
        if(dir >= 0) {
            FUNCTION_FILL_GRADIENT_RGB(leds,len,startcolor, endcolor);
        } else {
            FUNCTION_FILL_GRADIENT_RGB(leds + len + 1, (-len), endcolor, startcolor);
        }
        return *this;
    }

    /// Fill all of the LEDs with a smooth RGB gradient between three RGB colors.
    /// @param c1 the starting color in the gradient
    /// @param c2 the middle color for the gradient
    /// @param c3 the end color for the gradient
    /// @see ::fill_gradient_RGB(CRGB*, uint16_t, const CRGB&, const CRGB&, const CRGB&)
    inline CPixelView & fill_gradient_RGB(const PIXEL_TYPE & c1, const PIXEL_TYPE & c2, const PIXEL_TYPE &  c3) {
        if(dir >= 0) {
            FUNCTION_FILL_GRADIENT_RGB3(leds, len, c1, c2, c3);
        } else {
            FUNCTION_FILL_GRADIENT_RGB3(leds + len + 1, -len, c3, c2, c1);
        }
        return *this;
    }

    /// Fill all of the LEDs with a smooth RGB gradient between four RGB colors.
    /// @param c1 the starting color in the gradient
    /// @param c2 the first middle color for the gradient
    /// @param c3 the second middle color for the gradient
    /// @param c4 the end color for the gradient
    /// @see ::fill_gradient_RGB(CRGB*, uint16_t, const CRGB&, const CRGB&, const CRGB&, const CRGB&)
    inline CPixelView & fill_gradient_RGB(const PIXEL_TYPE & c1, const PIXEL_TYPE & c2, const PIXEL_TYPE & c3, const PIXEL_TYPE & c4) {
        if(dir >= 0) {
            FUNCTION_FILL_GRADIENT_RGB4(leds, len, c1, c2, c3, c4);
        } else {
            FUNCTION_FILL_GRADIENT_RGB4(leds + len + 1, -len, c4, c3, c2, c1);
        }
        return *this;
    }

    /// Destructively modifies all LEDs, blending in a given fraction of an overlay color
    /// @param overlay the color to blend in
    /// @param amountOfOverlay the fraction of overlay to blend in
    /// @see ::nblend(CRGB&, const CRGB&, fract8)
    inline CPixelView & nblend(const PIXEL_TYPE & overlay, fract8 amountOfOverlay) { for(iterator pixel = begin(), _end = end(); pixel != _end; ++pixel) { FUNCTION_NBLEND((*pixel), overlay, amountOfOverlay); } return *this; }

    /// Destructively blend another set of LEDs into this one
    /// @param rhs the set of LEDs to blend into this set
    /// @param amountOfOverlay the fraction of each color in the other set to blend in
    /// @see ::nblend(CRGB&, const CRGB&, fract8)
    inline CPixelView & nblend(const CPixelView & rhs, fract8 amountOfOverlay) { for(iterator pixel = begin(), rhspixel = rhs.begin(), _end = end(), rhs_end = rhs.end(); (pixel != _end) && (rhspixel != rhs_end); ++pixel, ++rhspixel) { FUNCTION_NBLEND((*pixel), (*rhspixel), amountOfOverlay); } return *this; }

    /// One-dimensional blur filter
    /// @param blur_amount the amount of blur to apply
    /// @note Only bringing in a 1d blur, not sure 2d blur makes sense when looking at sub arrays
    /// @see ::blur1d(CRGB*, uint16_t, fract8)
    inline CPixelView & blur1d(fract8 blur_amount) {
        if(dir >= 0) {
            FUNCTION_BLUR1D(leds, len, blur_amount);
        } else {
            FUNCTION_BLUR1D(leds + len + 1, -len, blur_amount);
        }
        return *this;
    }

    /// Destructively applies a gamma adjustment to all LEDs
    /// @param gamma the gamma value to apply
    /// @see ::napplyGamma_video(CRGB&, float)
    inline CPixelView & napplyGamma_video(float gamma) {
        if(dir >= 0) {
            FUNCTION_NAPPLY_GAMMA(leds, len, gamma);
        } else {
            FUNCTION_NAPPLY_GAMMA(leds + len + 1, -len, gamma);
        }
        return *this;
    }

    /// @copybrief CPixelView::napplyGamma_video(float)
    /// @param gammaR the gamma value to apply to the CRGB::red channel
    /// @param gammaG the gamma value to apply to the CRGB::green channel
    /// @param gammaB the gamma value to apply to the CRGB::blue channel
    /// @see ::napplyGamma_video(CRGB&, float, float, float)
    inline CPixelView & napplyGamma_video(float gammaR, float gammaG, float gammaB) {
        if(dir >= 0) {
            FUNCTION_NAPPLY_GAMMA_RGB(leds, len, gammaR, gammaG, gammaB);
        } else {
            FUNCTION_NAPPLY_GAMMA_RGB(leds + len + 1, -len, gammaR, gammaG, gammaB);
        }
        return *this;
    }

    /// @} Color Util Functions


    /// @name Iterator
    /// @{

    /// Iterator helper class for CPixelView
    /// @tparam the type of the LED array data
    /// @todo Make this a fully specified/proper iterator
    template <class T>
    class pixelset_iterator_base {
        T * leds;          ///< pointer to LED array
        const int8_t dir;  ///< direction of LED array, for incrementing the pointer

    public:
        /// Copy constructor
        FASTLED_FORCE_INLINE pixelset_iterator_base(const pixelset_iterator_base & rhs) : leds(rhs.leds), dir(rhs.dir) {}

        /// Base constructor
        /// @tparam the type of the LED array data
        /// @param _leds pointer to LED array
        /// @param _dir direction of LED array
        FASTLED_FORCE_INLINE pixelset_iterator_base(T * _leds, const char _dir) : leds(_leds), dir(_dir) {}

        FASTLED_FORCE_INLINE pixelset_iterator_base& operator++() { leds += dir; return *this; }  ///< Increment LED pointer in data direction
        FASTLED_FORCE_INLINE pixelset_iterator_base operator++(int) { pixelset_iterator_base tmp(*this); leds += dir; return tmp; }  ///< @copydoc operator++()

        FASTLED_FORCE_INLINE bool operator==(pixelset_iterator_base & other) const { return leds == other.leds; }    ///< Check if iterator is at the same position
        FASTLED_FORCE_INLINE bool operator!=(pixelset_iterator_base & other) const { return leds != other.leds; }  ///< Check if iterator is not at the same position

        FASTLED_FORCE_INLINE PIXEL_TYPE& operator*() const { return *leds; }  ///< Dereference operator, to get underlying pointer to the LEDs
    };

    typedef pixelset_iterator_base<PIXEL_TYPE> iterator;              ///< Iterator helper type for this class
    typedef pixelset_iterator_base<const PIXEL_TYPE> const_iterator;  ///< Const iterator helper type for this class

    iterator begin() { return iterator(leds, dir); }   ///< Makes an iterator instance for the start of the LED set
    iterator end() { return iterator(end_pos, dir); }  ///< Makes an iterator instance for the end of the LED set

    iterator begin() const { return iterator(leds, dir); }   ///< Makes an iterator instance for the start of the LED set, const qualified
    iterator end() const { return iterator(end_pos, dir); }  ///< Makes an iterator instance for the end of the LED set, const qualified

    const_iterator cbegin() const { return const_iterator(leds, dir); }   ///< Makes a const iterator instance for the start of the LED set, const qualified
    const_iterator cend() const { return const_iterator(end_pos, dir); }  ///< Makes a const iterator instance for the end of the LED set, const qualified

*/
