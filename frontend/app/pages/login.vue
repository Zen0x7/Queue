<template>
  <wrapper name="pages-login">
    <form class="form" id="form-login">
      <div class="form-group">
        <label for="email">Email</label>
        <input id="email" type="text" v-model="auth.form.email">
      </div>
      <div class="form-group">
        <label for="password">Password</label>
        <input id="password" type="text" v-model="auth.form.password">
      </div>
      <button type="button" @click="auth.attempt">Submit</button>

      {{ auth.bearer }}
    </form>
  </wrapper>
</template>

<script lang="ts" setup>
import { useAuth } from "~/stores/auth";

const auth = useAuth();
</script>