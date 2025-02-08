using System;
using System.Collections.Generic;
using System.Drawing;
using System.Runtime.InteropServices;
using System.Text;

using Louron;

namespace SandboxProject
{
    public class LightFlicker : Entity {

        PointLightComponent light;
        
        // Variables for controlling flicker effect
        public float minIntensity = 25.0f;
        public float maxIntensity = 30.0f;
        public float flickerSpeed = 5.0f;


        // Internal time tracker for flicker speed
        private float time;

        void OnStart()
        {
            light = GetComponent<PointLightComponent>();
        }

        void OnUpdate()
        {
            
            // Update time
            time += Time.deltaTime * flickerSpeed;

            // Calculate a new intensity using Perlin noise for smooth flickering
            float noise = LMath.PerlinNoise(time, 0.0f);

            // Map noise value to the desired intensity range
            light.intensity = Math.Clamp(LMath.Lerp(noise, minIntensity, maxIntensity), minIntensity, maxIntensity);
        }

        void OnFixedUpdate() 
        {

        }

        void OnDestroy()
        {

        }
    }
}