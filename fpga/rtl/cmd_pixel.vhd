library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.NUMERIC_STD.all;

entity cmd_pixel is
   port (
      i_reset_n        : in std_logic;
      i_clk            : in std_logic;
      i_data           : in std_logic_vector(7 downto 0);
      i_data_color     : in std_logic_vector(7 downto 0);
      i_data_rdy       : in std_logic; -- indication that new data is available
      i_writing        : in std_logic; -- indication that address write is in progress
      --
      o_busy           : out std_logic; -- indication that command is in progress
      o_need_more_data : out std_logic; -- indication that more data is needed
      o_address        : out unsigned(15 downto 0);  -- address to write the RGB values
      o_rgb            : out std_logic_vector(23 downto 0);
      o_alpha          : out unsigned(7 downto 0);
      o_write_clk      : out std_logic  -- on rising edge, address and rgb values are ready for writing
   );
end entity cmd_pixel;

-- pixel commands
-- 0x08,<red>,<green,<blue> - set brushcolor
-- 0x09,<x>,<y>,<alpha> - set pixel at coordinates with given alpha channel
-- 0x0A,<alpha> - set pixel at next coordinates with given alpha channel
-- 0x0B,<x>,<y>,<alpha> - merge pixel at coordinates with given alpha channel
-- 0x0C,<alpha> - merge at next coordinates with given alpha channel
-- 
-- commands 0x09 and 0x0A (set pixel) are not functioning yet
architecture cmd_pixel_arch of cmd_pixel is

   type T_STATE is ( CMD, ARG_X, ARG_Y, ARG_ALPHA, ARG_RED, ARG_GRN, ARG_BLU, WRITE_PIXEL, WAIT_WRITE_START, WAIT_WRITE_DONE );
   type T_MODE is ( SET, MERGE );

   signal s_x         : unsigned(7 downto 0);
   signal s_y         : unsigned(7 downto 0);
   signal s_write_clk : std_logic;
   signal s_state     : T_STATE;
   signal s_brush     : std_logic_vector(23 downto 0);
   signal s_alpha     : unsigned(7 downto 0);
   signal s_set       : std_logic;
   signal s_mode      : T_MODE;

begin
   cmd_pixel: process (i_reset_n, i_clk)
   variable v_red : unsigned(15 downto 0);
   variable v_grn : unsigned(15 downto 0);
   variable v_blu : unsigned(15 downto 0);
   begin
      if i_reset_n = '0' then
         s_state          <= CMD;
         s_write_clk      <= '0';
         o_busy           <= '0';
         o_need_more_data <= '0';
         s_x              <= to_unsigned(0, s_x'length);
         s_y              <= to_unsigned(0, s_y'length);
         s_brush          <= (others => '1');
         
      elsif rising_edge(i_clk) then

         case s_state is
         when CMD =>
            if i_data_rdy = '1' then
               if i_data = X"08" then
                  o_busy <= '1';
                  o_need_more_data <= '1';
                  s_state <= ARG_RED;
               elsif i_data = X"09" then
                  o_busy <= '1';
                  o_need_more_data <= '1';
                  s_mode <= SET;
                  s_state <= ARG_X;
               elsif i_data = X"0A" then
                  o_busy <= '1';
                  o_need_more_data <= '1';
                  s_x <= s_x + 1;
                  s_mode <= SET;
                  s_state <= ARG_ALPHA;
               elsif i_data = X"0B" then
                  o_busy <= '1';
                  o_need_more_data <= '1';
                  s_mode <= MERGE;
                  s_state <= ARG_X;
               elsif i_data = X"0C" then
                  o_busy <= '1';
                  o_need_more_data <= '1';
                  s_alpha <= unsigned(i_data);
                  s_x <= s_x + 1;
                  s_mode <= MERGE;
                  s_state <= ARG_ALPHA;
               end if;
            end if;
         when ARG_RED =>
            if i_data_rdy = '1' then
               s_brush(23 downto 16) <= i_data_color;
               s_state <= ARG_GRN;
            end if;
         when ARG_GRN =>
            if i_data_rdy = '1' then
               s_brush(15 downto 8) <= i_data_color;
               s_state <= ARG_BLU;
            end if;
         when ARG_BLU =>
            if i_data_rdy = '1' then
               s_brush(7 downto 0) <= i_data_color;
               o_need_more_data <= '0';
               o_busy <= '0';
               s_state <= CMD;
            end if;
         when ARG_X =>
            if i_data_rdy = '1' then
               s_x <= unsigned(i_data);
               s_state <= ARG_Y;
            end if;
         when ARG_Y =>
            if i_data_rdy = '1' then
               s_y <= unsigned(i_data);
               s_state <= ARG_ALPHA;
            end if;
         when ARG_ALPHA =>
            if i_data_rdy = '1' then
               s_alpha <= unsigned(i_data);            
               o_need_more_data <= '0';
               s_state <= WRITE_PIXEL;
            end if;
         when WRITE_PIXEL =>
            o_address <= s_y & s_x;
            if s_mode = SET then
               v_red := unsigned(s_brush(23 downto 16)) * s_alpha / 255;
               o_rgb(23 downto 16) <= std_logic_vector(v_red(7 downto 0));
               v_grn := unsigned(s_brush(15 downto 8)) * s_alpha / 255;
               o_rgb(15 downto 8) <= std_logic_vector(v_grn(7 downto 0));
               v_blu := unsigned(s_brush(7 downto 0)) * s_alpha / 255;
               o_rgb(7 downto 0) <= std_logic_vector(v_blu(7 downto 0));
               o_alpha <= to_unsigned(255, o_alpha'length);
            else
               o_rgb <= s_brush;
               o_alpha <= s_alpha;
            end if;
            s_write_clk <= '1';
            s_state <= WAIT_WRITE_START;
         when WAIT_WRITE_START =>
            if i_writing = '1' then
               s_write_clk <= '0';
               s_state <= WAIT_WRITE_DONE;
            end if;
         when WAIT_WRITE_DONE =>
            if i_writing = '0' then
               s_state <= CMD;
               o_busy <= '0';
            end if;
         end case;

         o_write_clk <= s_write_clk;

      end if;

   end process;
end architecture cmd_pixel_arch;



