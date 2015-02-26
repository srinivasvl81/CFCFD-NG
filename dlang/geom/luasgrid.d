/**
 * A Lua interface for the D sgrid (StructuredGrid) module.
 *
 * Authors: Rowan G. and Peter J.
 * Date: 2015-02-26
 */

module luasgrid;

// We cheat to get the C Lua headers by using LuaD.
import luad.all;
import luad.c.lua;
import luad.c.lauxlib;
import std.stdio;
import std.string;
import util.lua_service;
import univariatefunctions;
import geom;
import gpath;
import surface;
import sgrid;
import luasurface;

// Name of metatables
immutable string StructuredGrid2DMT = "StructuredGrid2D";
immutable string StructuredGrid3DMT = "StructuredGrid3D";

StructuredGrid checkStructuredGrid(lua_State* L, int index) {
    if ( isObjType(L, index, StructuredGrid2DMT) ) {
	return checkObj!(StructuredGrid, StructuredGrid2DMT)(L, index);
    }
    if ( isObjType(L, index, StructuredGrid3DMT) ) {
	return checkObj!(StructuredGrid, StructuredGrid3DMT)(L, index);
    }
    // if all else fails
    return null;
}

extern(C) int copyStructuredGrid(T, string MTname)(lua_State* L)
{
    // Sometimes it's convenient to get a copy of a function.
    auto f = checkObj!(T, MTname)(L, 1);
    pushObj!(T, MTname)(L, f);
    return 1;
}

/**
 * The Lua constructor for a StructuredGrid.
 *
 * Example construction in Lua:
 * grid = StructuredGrid2D:new{surf=someParametricSurface, niv=10, njv=10,
 *                             clusterf={cf_north, cf_east, cf_south, cf_west},
 *                             label="something"}
 * [TODO] 2D from ParametricVolume
 */
extern(C) int newStructuredGrid2D(lua_State* L)
{
    lua_remove(L, 1); // remove first agurment "this"
    int narg = lua_gettop(L);
    if ( narg == 0 || !lua_istable(L, 1) ) {
	string errMsg = `Error in call to StructuredGrid:new{}.;
A table containing arguments is expected, but no table was found.`;
	luaL_error(L, errMsg.toStringz);
    }
    lua_getfield(L, 1, "surf".toStringz);
    if ( lua_isnil(L, -1) ) {
	string errMsg = "Error in call to StructuredGrid:new{}. surf not found.";
	luaL_error(L, errMsg.toStringz);
    }
    ParametricSurface surf = checkSurface(L, -1);
    if (!surf) {
	string errMsg = "Error in call to StructuredGrid:new{}. surf not a ParametricSurface.";
	luaL_error(L, errMsg.toStringz);
    }
    string errMsgTmplt = `Error in call to StructuredGrid:new{}.
A valid value for '%s' was not found in list of arguments.
The value, if present, should be a number.`;
    int niv = getIntegerFromTable(L, 1, "niv", true, 0, true, format(errMsgTmplt, "niv"));
    int njv = getIntegerFromTable(L, 1, "njv", true, 0, true, format(errMsgTmplt, "njv"));
    // [TODO] include clustering functions.
    UnivariateFunction[] cfList = [new LinearFunction(), new LinearFunction(),
				   new LinearFunction(), new LinearFunction()];
    auto grid = new StructuredGrid(surf, niv, njv, cfList);
    return pushObj!(StructuredGrid, StructuredGrid2DMT)(L, grid);
}


void registerStructuredGrid(LuaState lua)
{
    auto L = lua.state;

    // Register the StructuredGrid2D object
    luaL_newmetatable(L, StructuredGrid2DMT.toStringz);
    
    /* metatable.__index = metatable */
    lua_pushvalue(L, -1); // duplicates the current metatable
    lua_setfield(L, -2, "__index");

    /* Register methods for use. */
    lua_pushcfunction(L, &newStructuredGrid2D);
    lua_setfield(L, -2, "new");
    lua_pushcfunction(L, &toStringObj!(StructuredGrid, StructuredGrid2DMT));
    lua_setfield(L, -2, "__tostring");
    lua_pushcfunction(L, &copyStructuredGrid!(StructuredGrid, StructuredGrid2DMT));
    lua_setfield(L, -2, "copy");

    lua_setglobal(L, StructuredGrid2DMT.toStringz);
} // end registerStructuredGrid()
    






