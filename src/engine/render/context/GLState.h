// #pragma once // internal header

// #include <glad/glad.h>


// /**
//  * @brief Simple data storage used for isolating OpenGL processes and settings
//  * 
//  */
// namespace kerf {

// class GLState {
// private:
//     // the current frame buffer
//     GLint fbo;

//     // the current viewport
//     GLint viewport[4];

//     // the current depth setting
//     GLboolean depthWasEnabled;

//     // safety boolean to prevent loading garbage data
//     bool saved = false;

// public:
//     /**
//      * @brief Construct a new GLState object
//      * 
//      */
//     GLState() = default;

//     /**
//      * @brief Destroy the GLState object
//      * 
//      */
//     ~GLState() = default;

//     /**
//      * @brief Saves the current OpenGL context settings
//      * 
//      */
//     void save();

//     /**
//      * @brief Sets the current OpenGL context settings to the values saved
//      * 
//      */
//     void restore() const;
// };

// } // namespace kerf