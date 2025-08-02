package com.medinova.app.presentation.theme

import androidx.compose.foundation.isSystemInDarkTheme
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.lightColorScheme
import androidx.compose.material3.darkColorScheme
import androidx.compose.runtime.Composable
import androidx.compose.ui.graphics.Color

private val LightColorScheme = lightColorScheme(
    primary = MedinovaPrimary,
    secondary = MedinovaSecondary,
    background = MedinovaBackground,
    surface = MedinovaSurface,
    error = MedinovaError,
    onPrimary = MedinovaOnPrimary,
    onSecondary = MedinovaOnSecondary,
    onBackground = MedinovaOnBackground,
    onSurface = MedinovaOnSurface,
    onError = MedinovaOnError
)

private val DarkColorScheme = darkColorScheme(
    primary = MedinovaPrimary,
    secondary = MedinovaSecondary,
    background = MedinovaDarkGray,
    surface = MedinovaDarkGray,
    error = MedinovaError,
    onPrimary = MedinovaOnPrimary,
    onSecondary = MedinovaOnSecondary,
    onBackground = Color.White,
    onSurface = Color.White,
    onError = MedinovaOnError
)

@Composable
fun MedinovaTheme(
    darkTheme: Boolean = isSystemInDarkTheme(),
    content: @Composable () -> Unit
) {
    val colorScheme = if (darkTheme) {
        DarkColorScheme
    } else {
        LightColorScheme
    }

    MaterialTheme(
        colorScheme = colorScheme,
        typography = Typography,
        shapes = Shapes,
        content = content
    )
}